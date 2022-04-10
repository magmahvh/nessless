#include "../options.hpp"
#include "../helpers/math.hpp"
#include <algorithm>

#include "resolver.hpp"
#include "../helpers/keybinds.hpp"
#include "ragebot.hpp"

#define delta(angle1, angle2) remainderf(fabsf(angle1 - angle2), 360.0f)
#define n(yaw) Math::NormalizeAngle(fabsf(yaw))

int Random(int to) {
	return rand() % to + 1;
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
		const bool is_slowing = pEnt->m_vecVelocity().Length2D() < 36;
		const bool is_crouching = pEnt->m_fFlags() & FL_DUCKING;

		int resolveYaw = pEnt->m_angEyeAngles().yaw;
		int resolveRoll = 0;
		int maxResolveYaw = 0;

		if (is_moving) maxResolveYaw = 60;
		else if (is_slowing) maxResolveYaw = 45;
		else maxResolveYaw = 15;

		switch (g_Ragebot->shots_fired % 4) {
		case 0:
			resolveYaw += maxResolveYaw;
			break;
		case 1:
			resolveYaw -= maxResolveYaw;
			break;
		case 2:
			resolveYaw += maxResolveYaw / 2;
			break;
		case 3:
			resolveYaw -= maxResolveYaw / 2;
			break;
		}

		if (KeyList::Get().rollresolver) {
			switch (g_Ragebot->shots_fired % 2) {
			case 0: resolveRoll = 45.f; break;
			case 1: resolveRoll = -45.f; break;
			}
		}

		pEnt->m_angEyeAngles().yaw = resolveYaw;
		pEnt->m_angEyeAngles().roll = resolveRoll;
	}
}