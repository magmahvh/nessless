#include "antiaim.hpp"
#include "fakelag.hpp"
#include "../helpers/keybinds.hpp"

int getPitch() {
	switch (g_Options.antiaim_pitch) {
	case 1:
		return 89;
	case 2:
		return -89;
	}
}

int getYaw() {
	switch (g_Options.antiaim_yaw) {
	case 1:
		return 180;
	case 2:
		return 0;
	case 3:
		return 90;
	case 4:
		return -90;
	}
}

void Antiaim::Run(CUserCmd* pCmd, bool& bSendPacket)
{
	if (!g_Options.antiaim)
		return;

	if ((pCmd->buttons & IN_USE) || (pCmd->buttons & IN_ATTACK) || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER)
		return;

	QAngle oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;
	bool invert = false;

	if (KeyList::Get().inverter) invert = true;
	
	if (!g_Options.fakelag) FakeLag::Get().Override(pCmd, bSendPacket, 2);
	 
	pitch = getPitch();
	yaw = getYaw();
	dsy = g_Options.antiaim_dsy;

	for (auto hk : Lua::Get().hooks->getHooks("antiaim"))
		hk.func();

	if (g_Options.antiaim_pitch != 0)
		pCmd->viewangles.pitch = pitch;

	if (g_Options.antiaim_yaw != 0)
		pCmd->viewangles.yaw += yaw;

	if (!bSendPacket) {
		pCmd->viewangles.yaw += invert ? dsy : -dsy;
		if (std::abs(pCmd->sidemove) < 5.0f) {
			if (pCmd->buttons & IN_DUCK) 
				pCmd->sidemove = pCmd->tick_count & 1 ? 3.25f : -3.25f;
			else
				pCmd->sidemove = pCmd->tick_count & 1 ? 1.1f : -1.1f;
		}
	}

	Math::CorrectMovement(oldAngle, pCmd, oldForward, oldSideMove);
}

/*void Antiaim::manage_local_fake_animstate()
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	static auto handle = g_LocalPlayer->GetRefEHandle();
	static auto spawn_time = g_LocalPlayer->m_flSpawnTime();
	auto fake_anim_state = g_LocalPlayer->GetPlayerAnimState();

	if (!fake_anim_state) return;

	bool allocate = (!fake_anim_state),
		change = (!allocate) && (g_LocalPlayer->GetRefEHandle() != handle),
		reset = (!allocate && !change) && (g_LocalPlayer->m_flSpawnTime() != spawn_time);

	if (change)
		free(fake_anim_state);

	if (reset)
	{
		g_LocalPlayer->ResetAnimationState(fake_anim_state);

		spawn_time = g_LocalPlayer->m_flSpawnTime();
	}

	if (allocate || change)
	{
		auto* state = reinterpret_cast<CCSGOPlayerAnimState*>(malloc(sizeof(CCSGOPlayerAnimState)));

		if (state != nullptr)
			g_LocalPlayer->CreateAnimationState(state);

		handle = g_LocalPlayer->GetRefEHandle();
		spawn_time = g_LocalPlayer->m_flSpawnTime();

		fake_anim_state = state;
	}
	else if (g_LocalPlayer->m_flSimulationTime() != g_LocalPlayer->m_flOldSimulationTime())
	{
		//std::memcpy(g_LocalPlayer->GetAnimOverlays(), g_LocalPlayer->GetAnimOverlays() + 13, sizeof(AnimationLayer) * 13);
		//std::copy(g_LocalPlayer->GetAnimOverlays(), g_LocalPlayer->GetAnimOverlays() + networked_layers.size(), networked_layers);

		auto backup_abs_angles = g_LocalPlayer->GetAbsAngles();
		auto backup_poses = g_LocalPlayer->m_flPoseParameter();

		g_LocalPlayer->UpdateAnimationState(fake_anim_state, angle);
		is_fakematrix_setup = g_LocalPlayer->SetupBones(fake_matrix, 128, 0x7FF00, g_GlobalVars->curtime);

		//std::memcpy(g_LocalPlayer->GetAnimOverlays(), g_LocalPlayer->GetAnimOverlays() + 13, sizeof(AnimationLayer) * 13);
		//std::copy(networked_layers.begin(), networked_layers.end(), g_LocalPlayer->GetAnimOverlays());

		g_LocalPlayer->m_flPoseParameter() = backup_poses;
		g_LocalPlayer->GetAbsAngles() = backup_abs_angles;
	}
}*/