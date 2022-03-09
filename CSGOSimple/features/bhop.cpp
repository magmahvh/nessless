#include "bhop.hpp"
#include "../options.hpp"

#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "bhopStrafe.hpp"
#include <filesystem>

void BunnyHop::OnCreateMove(CUserCmd* cmd)
{
	static bool jumped_last_tick = false;
	static bool should_fake_jump = false;
	if (!g_LocalPlayer)
		return;

	if (!g_LocalPlayer->IsAlive())
		return;

	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
		return;

	if (g_LocalPlayer->m_fFlags() & FL_INWATER)
		return;

	if(!jumped_last_tick && should_fake_jump) {
		should_fake_jump = false;
		cmd->buttons |= IN_JUMP;
	} else if(cmd->buttons & IN_JUMP) {
		if(g_LocalPlayer->m_fFlags() & FL_ONGROUND) {
			jumped_last_tick = true;
			should_fake_jump = true;
		} else {
			cmd->buttons &= ~IN_JUMP;
			jumped_last_tick = false;
		}
	} else {
		jumped_last_tick = false;
		should_fake_jump = false;
	}
}


static vec_t Normalize_y(vec_t ang)
{
	while (ang < -180.0f)
		ang += 360.0f;
	while (ang > 180.0f)
		ang -= 360.0f;
	return ang;

}

void BunnyHop::AutoStrafe(CUserCmd* cmd)
{
	if (cmd->sidemove != 0.0f || cmd->forwardmove != 0.0f || cmd->mousedx > 2) {
		return;
	}

	auto redian_to_degrees = [](float ang) -> float {
		return (ang * 180.0f) / M_PI;
	};

	auto normalize_ang = [](float ang) -> float {
		while (ang < -180.0f) ang += 360.0f;
		while (ang > 180.0f) ang -= 360.0f;

		return ang;
	};

	const auto vel = g_LocalPlayer->m_vecVelocity();
	const float y_vel = redian_to_degrees(atan2(vel.y, vel.x));
	const float diff_ang = normalize_ang(cmd->viewangles.yaw - y_vel);

	// Note that sidemove should be 400 for most Source games. CS:GO is an exclusion, we have it as 450!
	static float cl_sidespeed = g_CVar->FindVar("cl_sidespeed")->GetFloat();
	cmd->sidemove = (diff_ang > 0.0) ? -cl_sidespeed : cl_sidespeed;
	cmd->viewangles.yaw = normalize_ang(cmd->viewangles.yaw - diff_ang);
}