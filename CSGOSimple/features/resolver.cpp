#include "resolver.hpp"
#include "../options.hpp"
#include "../helpers/math.hpp"
#include <algorithm>

#define delta(angle1, angle2) remainderf(fabsf(angle1 - angle2), 360.0f)
#define n(yaw) Math::NormalizeAngle(fabsf(yaw))

float Bolbilize(float Yaw)
{
	if (Yaw > 180)
		Yaw -= (round(Yaw / 360) * 360.f);
	else if (Yaw < -180)
		Yaw += (round(Yaw / 360) * -360.f);

	return Yaw;
}

Vector CalcAngle69(Vector dst, Vector src)
{
	Vector angles;

	double delta[3] = { (src.x - dst.x), (src.y - dst.y), (src.z - dst.z) };
	double hyp = sqrt(delta[0] * delta[0] + delta[1] * delta[1]);
	angles.x = (float)(atan(delta[2] / hyp) * 180.0 / 3.14159265);
	angles.y = (float)(atanf(delta[1] / delta[0]) * 57.295779513082f);
	angles.z = 0.0f;

	if (delta[0] >= 0.0)
	{
		angles.y += 180.0f;
	}

	return angles;
}

void Resolver::Run(ClientFrameStage_t stage) {
	if (!g_Options.rage_enabled) return;
	if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected()) return;

	auto pLocal = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer());

	for (int i = 1; i < g_EntityList->GetHighestEntityIndex(); i++)
	{
		auto pEnt = (C_BasePlayer*)g_EntityList->GetClientEntity(i);

		if (!pEnt) continue;

		if (!pEnt->IsAlive() || !pEnt->IsPlayer() ||! g_LocalPlayer->IsAlive()) continue;

		CCSGOPlayerAnimState* animstate = pEnt->GetPlayerAnimState();

		if (!animstate) return;

		const bool is_moving = pEnt->m_vecVelocity().Length2D() > 0.1;
		const bool could_be_slowmo = pEnt->m_vecVelocity().Length2D() < 36;
		const bool is_crouching = pEnt->m_fFlags() & FL_DUCKING;

		int resolveYaw = 0;
		int resolveRoll = 0;

		float fl_eye_yaw = pEnt->m_angEyeAngles().yaw;
		float fl_lby_yaw = pEnt->m_flLowerBodyYawTarget();
		float fl_desync_delta = delta(fl_eye_yaw, pEnt->m_angEyeAngles().yaw);

		/* setup target side */
		float fl_left_yaw = n(fl_eye_yaw - 60.0);
		float fl_right_yaw = n(fl_eye_yaw + 60.0);

		/* setup low delta */
		float fl_left_low_delta = n(fl_lby_yaw - 35.0);
		float fl_right_low_delta = n(fl_lby_yaw + 35.0);

		if (fl_desync_delta < 35) {
			switch (pLocal->m_iShotsFired() % 3) {
			case 0:
				resolveYaw = storedBrute[pEnt->EntIndex()];
				break;
			case 1:
				resolveYaw = fl_left_low_delta;
				storedBrute[pEnt->EntIndex()] = fl_left_low_delta;
				break;
			case 2:
				resolveYaw = fl_right_low_delta;
				storedBrute[pEnt->EntIndex()] = fl_right_low_delta;
				break;
			}
		}

		if (fabs(fl_desync_delta) > 60.0) {
			if (fabs(fl_desync_delta) > 0) {
				resolveYaw = fl_left_yaw;
			}
			else {
				resolveYaw = fl_right_yaw;
			}
		}
		else if (fabs(fl_desync_delta) < -60.0) {
			if (fabs(fl_desync_delta) > 0) {
				resolveYaw = fl_right_yaw;
			}
			else {
				resolveYaw = fl_left_yaw;
			}
		}

		if (GetAsyncKeyState(g_Options.roll_resolver)) {
			switch (g_LocalPlayer->m_iShotsFired() % 2) {
			case 0: resolveRoll = 45.f; break;
			case 1: resolveRoll = -45.f; break;
			}
		}

		pEnt->m_angEyeAngles().yaw = resolveYaw;
		pEnt->m_angEyeAngles().roll = resolveRoll;
	}
}