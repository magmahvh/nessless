#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../valve_sdk/sdk.hpp"

#include <DirectXMath.h>

#define RAD2DEG(x) DirectX::XMConvertToDegrees(x)
#define DEG2RAD(x) DirectX::XMConvertToRadians(x)
#define M_PI 3.14159265358979323846
#define PI_F	((float)(M_PI)) 

namespace Math
{
	inline float FASTSQRT(float x)
	{
		unsigned int i = *(unsigned int*)&x;

		i += 127 << 23;
		// approximation of square root
		i >>= 1;
		return *(float*)&i;
	}
	void NormalizeAnglesvec(Vector& angles);
	void NormalizeAngles(QAngle& angles);
	float VectorDistance(const Vector& v1, const Vector& v2);
	QAngle CalcAngle(const Vector& src, const Vector& dst);
	float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle);
	template<class T>
	void Normalize3(T& vec)
	{
		for (auto i = 0; i < 2; i++) {
			while (vec[i] < -180.0f) vec[i] += 360.0f;
			while (vec[i] >  180.0f) vec[i] -= 360.0f;
		}
		vec[2] = 0.f;
	}
    void ClampAngles(QAngle& angles);
	void FixAngles(QAngle& angles);
    void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out);
    void AngleVectors(const QAngle &angles, Vector& forward);
    void AngleVectors(const QAngle &angles, Vector& forward, Vector& right, Vector& up);
    void VectorAngles(const Vector& forward, QAngle& angles);
    bool WorldToScreen(const Vector& in, Vector& out);
	void CorrectMovement(CUserCmd* cmd, QAngle wish_angle, QAngle old_angles);
	void CorrectMovement(QAngle vOldAngles, CUserCmd* pCmd, float fOldForward, float fOldSidemove);
	void vector_angles(const Vector& forward, Vector& angles);
	void angle_vectors(const Vector& angles, Vector& forward);

}