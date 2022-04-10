#include "Math.hpp"
#include <array>
#include <filesystem>

static const float invtwopi = 0.1591549f;
static const float twopi = 6.283185f;
static const float threehalfpi = 4.7123889f;
static const float pi = 3.141593f;
static const float halfpi = 1.570796f;
static const __m128 signmask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));

static const __declspec(align(16)) float null[4] = { 0.f, 0.f, 0.f, 0.f };
static const __declspec(align(16)) float _pi2[4] = { 1.5707963267948966192f, 1.5707963267948966192f, 1.5707963267948966192f, 1.5707963267948966192f };
static const __declspec(align(16)) float _pi[4] = { 3.141592653589793238f, 3.141592653589793238f, 3.141592653589793238f, 3.141592653589793238f };

typedef __declspec(align(16)) union
{
    float f[4];
    __m128 v;
} m128;

__forceinline __m128 sqrt_ps(const __m128 squared)
{
    return _mm_sqrt_ps(squared);
}

__forceinline __m128 cos_52s_ps(const __m128 x)
{
    const auto c1 = _mm_set1_ps(0.9999932946f);
    const auto c2 = _mm_set1_ps(-0.4999124376f);
    const auto c3 = _mm_set1_ps(0.0414877472f);
    const auto c4 = _mm_set1_ps(-0.0012712095f);
    const auto x2 = _mm_mul_ps(x, x);
    return _mm_add_ps(c1, _mm_mul_ps(x2, _mm_add_ps(c2, _mm_mul_ps(x2, _mm_add_ps(c3, _mm_mul_ps(c4, x2))))));
}

__forceinline void sincos_ps(__m128 angle, __m128* sin, __m128* cos) {
    const auto anglesign = _mm_or_ps(_mm_set1_ps(1.f), _mm_and_ps(signmask, angle));
    angle = _mm_andnot_ps(signmask, angle);
    angle = _mm_sub_ps(angle, _mm_mul_ps(_mm_cvtepi32_ps(_mm_cvttps_epi32(_mm_mul_ps(angle, _mm_set1_ps(invtwopi)))), _mm_set1_ps(twopi)));

    auto cosangle = angle;
    cosangle = _mm_xor_ps(cosangle, _mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(halfpi)), _mm_xor_ps(cosangle, _mm_sub_ps(_mm_set1_ps(pi), angle))));
    cosangle = _mm_xor_ps(cosangle, _mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(pi)), signmask));
    cosangle = _mm_xor_ps(cosangle, _mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(threehalfpi)), _mm_xor_ps(cosangle, _mm_sub_ps(_mm_set1_ps(twopi), angle))));

    auto result = cos_52s_ps(cosangle);
    result = _mm_xor_ps(result, _mm_and_ps(_mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(halfpi)), _mm_cmplt_ps(angle, _mm_set1_ps(threehalfpi))), signmask));
    *cos = result;

    const auto sinmultiplier = _mm_mul_ps(anglesign, _mm_or_ps(_mm_set1_ps(1.f), _mm_and_ps(_mm_cmpgt_ps(angle, _mm_set1_ps(pi)), signmask)));
    *sin = _mm_mul_ps(sinmultiplier, sqrt_ps(_mm_sub_ps(_mm_set1_ps(1.f), _mm_mul_ps(result, result))));
}

namespace Math
{
    float fast_sin(float x)
    {
        x *= float(0.159155);
        x -= floor(x);
        const auto xx = x * x;
        auto y = -6.87897;
        y = y * xx + 33.7755;
        y = y * xx - 72.5257;
        y = y * xx + 80.5874;
        y = y * xx - 41.2408;
        y = y * xx + 6.28077;
        return float(x * y);
    }

    inline float fast_cos(const float x)
    {
        return fast_sin(x + 1.5708f);
    }
    inline void sin_cos(const float radians, float* sine, float* cosine)
    {
#ifdef QUICK_MATH
        * sine = fast_sin(radians);
        *cosine = fast_cos(radians);
#else
        * sine = sin(radians);
        *cosine = cos(radians);
#endif
    }

    void angle_matrix(const Vector& angles, matrix3x4_t& matrix)
    {
        float sr, sp, sy, cr, cp, cy;

        sin_cos(DEG2RAD(angles.x), &sp, &cp);
        sin_cos(DEG2RAD(angles.y), &sy, &cy);
        sin_cos(DEG2RAD(angles.z), &sr, &cr);

        // matrix = (YAW * PITCH) * ROLL
        matrix[0][0] = cp * cy;
        matrix[1][0] = cp * sy;
        matrix[2][0] = -sp;

        const auto crcy = cr * cy;
        const auto crsy = cr * sy;
        const auto srcy = sr * cy;
        const auto srsy = sr * sy;
        matrix[0][1] = sp * srcy - crsy;
        matrix[1][1] = sp * srsy + crcy;
        matrix[2][1] = sr * cp;

        matrix[0][2] = (sp * crcy + srsy);
        matrix[1][2] = (sp * crsy - srcy);
        matrix[2][2] = cr * cp;

        matrix[0][3] = 0.0f;
        matrix[1][3] = 0.0f;
        matrix[2][3] = 0.0f;
    }
    void matrix_set_column(const Vector& in, int column, matrix3x4_t& out)
    {
        out[0][column] = in.x;
        out[1][column] = in.y;
        out[2][column] = in.z;
    }

    void angle_matrix(const Vector& angles, const Vector& position, matrix3x4_t& matrix)
    {
        angle_matrix(angles, matrix);
        matrix_set_column(position, 3, matrix);
    }
    __forceinline  matrix3x4_t angle_matrix(const Vector angles)
    {
        matrix3x4_t result{};

        m128 angle, sin, cos;
        angle.f[0] = DEG2RAD(angles.x);
        angle.f[1] = DEG2RAD(angles.y);
        angle.f[2] = DEG2RAD(angles.z);
        sincos_ps(angle.v, &sin.v, &cos.v);

        result[0][0] = cos.f[0] * cos.f[1];
        result[1][0] = cos.f[0] * sin.f[1];
        result[2][0] = -sin.f[0];

        const auto crcy = cos.f[2] * cos.f[1];
        const auto crsy = cos.f[2] * sin.f[1];
        const auto srcy = sin.f[2] * cos.f[1];
        const auto srsy = sin.f[2] * sin.f[1];

        result[0][1] = sin.f[0] * srcy - crsy;
        result[1][1] = sin.f[0] * srsy + crcy;
        result[2][1] = sin.f[2] * cos.f[0];

        result[0][2] = sin.f[0] * crcy + srsy;
        result[1][2] = sin.f[0] * crsy - srcy;
        result[2][2] = cos.f[2] * cos.f[0];

        return result;
    }

    matrix3x4_t angle_matrix(const Vector angle, const Vector pos)
    {
        auto result = angle_matrix(angle);
        result[0][3] = pos.x;
        result[1][3] = pos.y;
        result[2][3] = pos.z;
        return result;
    }
    //------------------------------------------------------------------
    void NormalizeAnglesvec(Vector& angles)
    {
        for (auto i = 0; i < 3; i++) {
            while (angles[i] < -180.0f) angles[i] += 360.0f;
            while (angles[i] > 180.0f) angles[i] -= 360.0f;
        }
    }
    void NormalizeAngles(QAngle& angles)
    {
        for (auto i = 0; i < 3; i++) {
            while (angles[i] < -180.0f) angles[i] += 360.0f;
            while (angles[i] > 180.0f) angles[i] -= 360.0f;
        }
    }
	//--------------------------------------------------------------------------------
	float VectorDistance(const Vector& v1, const Vector& v2)
	{
		return FASTSQRT(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
	}
	//--------------------------------------------------------------------------------
	QAngle CalcAngle(const Vector& src, const Vector& dst)
	{
		QAngle vAngle;
		Vector delta((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));
		double hyp = sqrt(delta.x*delta.x + delta.y*delta.y);

		vAngle.pitch = float(atanf(float(delta.z / hyp)) * 57.295779513082f);
		vAngle.yaw = float(atanf(float(delta.y / delta.x)) * 57.295779513082f);
		vAngle.roll = 0.0f;

		if (delta.x >= 0.0)
			vAngle.yaw += 180.0f;

		return vAngle;
	}
	//--------------------------------------------------------------------------------
	float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle)
	{
		Vector ang, aim;

		AngleVectors(viewAngle, aim);
		AngleVectors(aimAngle, ang);

		auto res = RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
		if (std::isnan(res))
			res = 0.f;
		return res;
	}
    //--------------------------------------------------------------------------------
    void ClampAngles(QAngle& angles)
    {
        if(angles.pitch > 89.0f) angles.pitch = 89.0f;
        else if(angles.pitch < -89.0f) angles.pitch = -89.0f;

        if(angles.yaw > 180.0f) angles.yaw = 180.0f;
        else if(angles.yaw < -180.0f) angles.yaw = -180.0f;

        angles.roll = 0;
    }
    //--------------------------------------------------------------------------------
    void FixAngles(QAngle& angles)
    {
        Normalize3(angles);
        ClampAngles(angles);
    }
    //--------------------------------------------------------------------------------
    void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out)
    {
        out[0] = in1.Dot(in2[0]) + in2[0][3];
        out[1] = in1.Dot(in2[1]) + in2[1][3];
        out[2] = in1.Dot(in2[2]) + in2[2][3];
    }
    //--------------------------------------------------------------------------------
    void AngleVectors(const QAngle &angles, Vector& forward)
    {
        float	sp, sy, cp, cy;

        DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles[0]));
        DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles[1]));

        forward.x = cp*cy;
        forward.y = cp*sy;
        forward.z = -sp;
    }
    //--------------------------------------------------------------------------------
    void AngleVectors(const QAngle &angles, Vector& forward, Vector& right, Vector& up)
    {
        float sr, sp, sy, cr, cp, cy;

        DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles[0]));
        DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles[1]));
        DirectX::XMScalarSinCos(&sr, &cr, DEG2RAD(angles[2]));

        forward.x = (cp * cy);
        forward.y = (cp * sy);
        forward.z = (-sp);
        right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
        right.y = (-1 * sr * sp * sy + -1 * cr *  cy);
        right.z = (-1 * sr * cp);
        up.x = (cr * sp * cy + -sr*-sy);
        up.y = (cr * sp * sy + -sr*cy);
        up.z = (cr * cp);
    }
    //--------------------------------------------------------------------------------
    void VectorAngles(const Vector& forward, QAngle& angles)
    {
        float	tmp, yaw, pitch;

        if(forward[1] == 0 && forward[0] == 0) {
            yaw = 0;
            if(forward[2] > 0)
                pitch = 270;
            else
                pitch = 90;
        } else {
            yaw = (atan2(forward[1], forward[0]) * 180 / DirectX::XM_PI);
            if(yaw < 0)
                yaw += 360;

            tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
            pitch = (atan2(-forward[2], tmp) * 180 / DirectX::XM_PI);
            if(pitch < 0)
                pitch += 360;
        }

        angles[0] = pitch;
        angles[1] = yaw;
        angles[2] = 0;
    }
    //--------------------------------------------------------------------------------
    static bool screen_transform(const Vector& in, Vector& out)
    {
        static auto& w2sMatrix = g_EngineClient->WorldToScreenMatrix();

        out.x = w2sMatrix.m[0][0] * in.x + w2sMatrix.m[0][1] * in.y + w2sMatrix.m[0][2] * in.z + w2sMatrix.m[0][3];
        out.y = w2sMatrix.m[1][0] * in.x + w2sMatrix.m[1][1] * in.y + w2sMatrix.m[1][2] * in.z + w2sMatrix.m[1][3];
        out.z = 0.0f;

        float w = w2sMatrix.m[3][0] * in.x + w2sMatrix.m[3][1] * in.y + w2sMatrix.m[3][2] * in.z + w2sMatrix.m[3][3];

        if(w < 0.001f) {
            out.x *= 100000;
            out.y *= 100000;
            return false;
        }

        out.x /= w;
        out.y /= w;

        return true;
    }
    //--------------------------------------------------------------------------------
    bool WorldToScreen(const Vector& in, Vector& out)
    {
        if(screen_transform(in, out)) {
            int w, h;
            g_EngineClient->GetScreenSize(w, h);

            out.x = (w / 2.0f) + (out.x * w) / 2.0f;
            out.y = (h / 2.0f) - (out.y * h) / 2.0f;

            return true;
        }
        return false;
    }
    //--------------------------------------------------------------------------------
    void CorrectMovement(CUserCmd* cmd, QAngle wish_angle, QAngle old_angles)
    {
        if (old_angles.pitch != wish_angle.pitch || old_angles.yaw != wish_angle.yaw || old_angles.roll != wish_angle.roll)
        {
            Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

            auto viewangles = old_angles;
            auto movedata = Vector(cmd->forwardmove, cmd->sidemove, cmd->upmove);
            viewangles.Normalize();

            if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && viewangles.roll != 0.f)
                movedata.y = 0.f;

            AngleVectors(wish_angle, wish_forward, wish_right, wish_up);
            AngleVectors(viewangles, cmd_forward, cmd_right, cmd_up);

            auto v8 = sqrt(wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y), v10 = sqrt(wish_right.x * wish_right.x + wish_right.y * wish_right.y), v12 = sqrt(wish_up.z * wish_up.z);

            Vector wish_forward_norm(1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f),
                wish_right_norm(1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f),
                wish_up_norm(0.f, 0.f, 1.0f / v12 * wish_up.z);

            auto v14 = sqrt(cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y), v16 = sqrt(cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y), v18 = sqrt(cmd_up.z * cmd_up.z);

            Vector cmd_forward_norm(1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f),
                cmd_right_norm(1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f),
                cmd_up_norm(0.f, 0.f, 1.0f / v18 * cmd_up.z);

            auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

            Vector correct_movement;
            correct_movement.x = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25 + (cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28) + (cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27);
            correct_movement.y = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25 + (cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28) + (cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27);
            correct_movement.z = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25 + (cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28) + (cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27);

            correct_movement.x = std::clamp(correct_movement.x, -450.f, 450.f);
            correct_movement.y = std::clamp(correct_movement.y, -450.f, 450.f);
            correct_movement.z = std::clamp(correct_movement.z, -320.f, 320.f);

            cmd->forwardmove = correct_movement.x;
            cmd->sidemove = correct_movement.y;
            cmd->upmove = correct_movement.z;

            cmd->buttons &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);
            if (cmd->sidemove != 0.0) {
                if (cmd->sidemove <= 0.0)
                    cmd->buttons |= IN_MOVELEFT;
                else
                    cmd->buttons |= IN_MOVERIGHT;
            }

            if (cmd->forwardmove != 0.0) {
                if (cmd->forwardmove <= 0.0)
                    cmd->buttons |= IN_BACK;
                else
                    cmd->buttons |= IN_FORWARD;
            }
        }
    }

    void CorrectMovement(QAngle vOldAngles, CUserCmd* pCmd, float fOldForward, float fOldSidemove)
    {
        // side/forward move correction
        float deltaView;
        float f1;
        float f2;

        if (vOldAngles.yaw < 0.f)
            f1 = 360.0f + vOldAngles.yaw;
        else
            f1 = vOldAngles.yaw;

        if (pCmd->viewangles.yaw < 0.0f)
            f2 = 360.0f + pCmd->viewangles.yaw;
        else
            f2 = pCmd->viewangles.yaw;

        if (f2 < f1)
            deltaView = abs(f2 - f1);
        else
            deltaView = 360.0f - abs(f1 - f2);

        deltaView = 360.0f - deltaView;

        pCmd->forwardmove = cos(DEG2RAD(deltaView)) * fOldForward + cos(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
        pCmd->sidemove = sin(DEG2RAD(deltaView)) * fOldForward + sin(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
    }


    void vector_angles(const Vector& forward, Vector& angles)
    {
        Vector view;

        if (!forward[0] && !forward[1])
        {
            view[0] = 0.0f;
            view[1] = 0.0f;
        }
        else
        {
            view[1] = atan2(forward[1], forward[0]) * 180.0f / M_PI;

            if (view[1] < 0.0f)
                view[1] += 360.0f;

            view[2] = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
            view[0] = atan2(forward[2], view[2]) * 180.0f / M_PI;
        }

        angles[0] = -view[0];
        angles[1] = view[1];
        angles[2] = 0.f;
    }

    void angle_vectors(const Vector& angles, Vector& forward)
    {
        float sp, sy, cp, cy;

        sy = sin(DEG2RAD(angles[1]));
        cy = cos(DEG2RAD(angles[1]));

        sp = sin(DEG2RAD(angles[0]));
        cp = cos(DEG2RAD(angles[0]));

        forward.x = cp * cy;
        forward.y = cp * sy;
        forward.z = -sp;
    }

    int TIME_TO_TICKS(int dt)
    {
        return ((int)(0.5f + (float)(dt) / g_GlobalVars->interval_per_tick));
    }
    int TICKS_TO_TIME(int t)
    {
        return (g_GlobalVars->interval_per_tick * (t));
    }

}
