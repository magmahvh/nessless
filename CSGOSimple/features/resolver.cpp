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
	if (!g_Options.rage_enabled) return;

	int brute_angle_normal[] = {58, -58, 45, -45, 30, -30, 15, -15};
	int brute_angle_roll[] = { 45, -45, 25, -25, 10, -10 };

	Vector vEyeAngles = player->GetEyePos();
	float flLowerBody = remainderf(player->m_flLowerBodyYawTarget(), 360.f);

	const bool is_moving = player->m_vecVelocity().Length2D() > 0.1;
	const bool could_be_slowmo = player->m_vecVelocity().Length2D() < 36;
	const bool is_crouching = player->m_fFlags() & FL_DUCKING;

	switch (g_LocalPlayer->m_iShotsFired() % 6) {
	case 0: player->m_angEyeAngles().yaw = -58.f; break;
	case 1: player->m_angEyeAngles().yaw = 58.f; break;
	case 2: player->m_angEyeAngles().yaw = -30.f; break;
	case 3: player->m_angEyeAngles().yaw = 30.f; break;
	case 4: player->m_angEyeAngles().yaw = -10.f; break;
	case 5: player->m_angEyeAngles().yaw = 10.f; break;
	}

	if (GetAsyncKeyState(g_Options.roll_resolver)) {
		switch (g_LocalPlayer->m_iShotsFired() % 4) {
		case 0: player->m_angEyeAngles().roll = 45.f; break;
		case 1: player->m_angEyeAngles().roll = -45.f; break;
		case 2: player->m_angEyeAngles().roll = 25.f; break;
		case 3: player->m_angEyeAngles().roll = -25.f; break;
		}
	}
}