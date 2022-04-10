#include "animationfix.hpp"
#include "../helpers/math.hpp"
#include "antiaim.hpp"

void AnimFix::LocalAnimation(CCSGOPlayerAnimState* animstate) { // FRAME_RENDER_START

	auto local = g_LocalPlayer;

	if (!local || !animstate)
		return;

	local->UpdateAnimationState(animstate, Antiaim::Get().angle); // in createmove save viewangles when sending packet

	animstate->m_flGoalFeetYaw = g_LocalPlayer->GetEyePos().y;  // same shit
}