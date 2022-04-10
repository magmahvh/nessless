#include "../options.hpp"
#include "../helpers/math.hpp"
#include <algorithm>

#include "resolver.hpp"
#include "../helpers/keybinds.hpp"
#include "ragebot.hpp"

#define delta(angle1, angle2) remainderf(fabsf(angle1 - angle2), 360.0f)
#define n(yaw) Math::NormalizeAngle(fabsf(yaw))

float GetBackwardYaw(C_BaseEntity* player) {
	return Math::CalcAngle(g_LocalPlayer->abs_origin(), player->GetRenderOrigin()).yaw;
}


int DetectSide(C_BasePlayer* player)
{
	Vector src3D, dst3D, forward, right, up, src, dst;
	float back_two, right_two, left_two;
	trace_t tr;
	Ray_t ray, ray2, ray3, ray4, ray5;
	CTraceFilter filter;

	Math::angle_vectors(Vector(0, GetBackwardYaw(player), 0), forward);

	filter.pSkip = player;
	src3D = player->m_vecAngles();
	dst3D = src3D + (forward * 384); //Might want to experiment with other numbers, incase you don't know what the number does, its how far the trace will go. Lower = shorter.

	ray.Init(src3D, dst3D);
	g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);
	back_two = (tr.endpos - tr.startpos).Length();

	ray2.Init(src3D + right * 35, dst3D + right * 35);
	g_EngineTrace->TraceRay(ray2, MASK_SHOT, &filter, &tr);
	right_two = (tr.endpos - tr.startpos).Length();

	ray3.Init(src3D - right * 35, dst3D - right * 35);
	g_EngineTrace->TraceRay(ray3, MASK_SHOT, &filter, &tr);
	left_two = (tr.endpos - tr.startpos).Length();

	if (left_two > right_two) {
		return 1;
	}
	else if (right_two > left_two) {
		return -1;
	}
	else
		return 0;
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

		const bool is_run = pEnt->m_vecVelocity().Length2D() > 180;
		const bool is_moving = pEnt->m_vecVelocity().Length2D() > 0.1;
		const bool is_slowing = pEnt->m_vecVelocity().Length2D() < 36;
		const bool is_crouching = pEnt->m_fFlags() & FL_DUCKING;
		const bool is_air = !pEnt->m_fFlags() & FL_ONGROUND;

		int resolveYaw = pEnt->m_angEyeAngles().yaw;
		int resolveRoll = 0;
		int maxResolveYaw = 0;

		if (is_slowing) maxResolveYaw = 45;
		else if (is_air || is_run) maxResolveYaw = 15;
		else maxResolveYaw = 60;

		switch (g_Ragebot->shots_fired % 4) {
		case 0:
			resolveYaw += (maxResolveYaw) * DetectSide(pEnt);
			break;
		case 1:
			resolveYaw += (maxResolveYaw / 2) * DetectSide(pEnt);
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