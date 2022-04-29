#include "animationfix.hpp"
#include "../helpers/math.hpp"
#include "antiaim.hpp"
#include "../helpers/keybinds.hpp"

void AnimFix::LocalAnimation(CCSGOPlayerAnimState* animstate) { // FRAME_RENDER_START

	auto local = g_LocalPlayer;

	if (!local || !animstate)
		return;

	local->UpdateAnimationState(animstate, Antiaim::Get().angle); // in createmove save viewangles when sending packet

	if (KeyList::Get().inverter)
		local->m_angEyeAngles().yaw += Antiaim::Get().dsy;
	else
		local->m_angEyeAngles().yaw -= Antiaim::Get().dsy;
}