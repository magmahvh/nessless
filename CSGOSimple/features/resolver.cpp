#include "../options.hpp"
#include "../helpers/math.hpp"
#include <algorithm>

#include "resolver.hpp"
#include "../helpers/keybinds.hpp"
#include "ragebot.hpp"

#define delta(angle1, angle2) remainderf(fabsf(angle1 - angle2), 360.0f)
#define n(yaw) Math::NormalizeAngle(fabsf(yaw))

int resolveDegree(int side, C_BasePlayer* e) {
	switch (side)
	{
	case DSY_ORIGINAL:
		return e->m_angEyeAngles().yaw;
		break;
	case DSY_ZERO:
		return e->m_angEyeAngles().yaw;
		break;
	case DSY_FIRST:
		return e->m_angEyeAngles().yaw + 60;
		break;
	case DSY_SECOND:
		return e->m_angEyeAngles().yaw - 60;
		break;
	case DSY_LOW_FIRST:
		return e->m_angEyeAngles().yaw + 30;
		break;
	case DSY_LOW_SECOND:
		return e->m_angEyeAngles().yaw - 30;
		break;
	}
}

void Resolver::Run(ClientFrameStage_t stage) {
	if (!g_Options.rage_enabled) return;
	if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected()) return;

	for (int i = 1; i < g_EntityList->GetHighestEntityIndex(); i++)
	{
		auto pEnt = (C_BasePlayer*)g_EntityList->GetClientEntity(i);

		if (!pEnt) continue;

		if (!pEnt->IsAlive() || !pEnt->IsPlayer() || !g_LocalPlayer->IsAlive()) continue;

		if (pEnt == g_LocalPlayer) continue;

		CCSGOPlayerAnimState* animstate = pEnt->GetPlayerAnimState();

		if (!animstate) return;

		static bool was_first_bruteforce = false;
		static bool was_second_bruteforce = false;
		int resolveSide = DSY_ORIGINAL;
		int resolveRoll = 0;

		switch (g_Ragebot->shots_fired[pEnt->EntIndex()] % 4) {
		case 0:
			resolveSide = DSY_LOW_FIRST;

			was_first_bruteforce = false;
			was_second_bruteforce = false;
			break;
		case 1:
			resolveSide = was_second_bruteforce ? DSY_ZERO : DSY_SECOND;

			was_second_bruteforce = true;
			break;
		case 2:
			resolveSide = was_first_bruteforce ? DSY_ZERO : DSY_FIRST;

			was_first_bruteforce = true;
			break;
		case 3:
			switch (resolveSide) {
			case DSY_LOW_FIRST:
				resolveSide = DSY_LOW_SECOND;
				break;
			case DSY_LOW_SECOND:
				resolveSide = DSY_FIRST;
				break;
			}
			break;
		}

		if (KeyList::Get().rollresolver) {
			switch (g_Ragebot->shots_fired[pEnt->EntIndex()] % 2) {
			case 0: resolveRoll = 45.f; break;
			case 1: resolveRoll = -45.f; break;
			}
		}

		pEnt->m_angEyeAngles().yaw = resolveDegree(resolveSide, pEnt);
		pEnt->m_angEyeAngles().roll = resolveRoll;
	}
}