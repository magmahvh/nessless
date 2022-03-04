#include "resolver.hpp"
#include "../options.hpp"
#include "../helpers/math.hpp"
#include <algorithm>

float flAngleMod(float flAngle)
{
	return((360.0f / 65536.0f) * ((int32_t)(flAngle * (65536.0f / 360.0f)) & 65535));
}

float ApproachAngle(float flTarget, float flValue, float flSpeed)
{
	flTarget = flAngleMod(flTarget);
	flValue = flAngleMod(flValue);

	float delta = flTarget - flValue;

	// Speed is assumed to be positive
	if (flSpeed < 0)
		flSpeed = -flSpeed;

	if (delta < -180)
		delta += 360;
	else if (delta > 180)
		delta -= 360;

	if (delta > flSpeed)
		flValue += flSpeed;
	else if (delta < -flSpeed)
		flValue -= flSpeed;
	else
		flValue = flTarget;

	return flValue;
}

void Resolver::Resolve(C_BasePlayer* player) {
    if (g_Options.rage_enabled)
    {
        Vector vEyeAngles = player->GetEyePos();
        float flLowerBody = remainderf(player->m_flLowerBodyYawTarget(), 360.f);

        if (flLowerBody - remainderf(vEyeAngles.y, 360.f) >= 60.f)
			if (g_LocalPlayer->m_iShotsFired() % 2)
				player->m_angEyeAngles().yaw = 58.f;
			else
				player->m_angEyeAngles().yaw = -58.f;
    }
}