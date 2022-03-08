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
	static AutoStrafer Strafer;

	static float move = 450;
	float s_move = move * 0.5065f;
	if (g_LocalPlayer->m_nMoveType() & (MOVETYPE_NOCLIP | MOVETYPE_LADDER))
		return;
	if (cmd->buttons & (IN_FORWARD | IN_MOVERIGHT | IN_MOVELEFT | IN_BACK))
		return;

	if (cmd->buttons & IN_JUMP | !(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
	{
		if (g_LocalPlayer->m_vecVelocity().Length2D() == 0 && (cmd->forwardmove == 0 && cmd->sidemove == 0))
		{
			cmd->forwardmove = 450.f;
		}
		else if (cmd->forwardmove == 0 && cmd->sidemove == 0)
		{
			if (cmd->mousedx > 0 || cmd->mousedx > -0)
			{
				cmd->sidemove = cmd->mousedx < 0.f ? -450.f : 450.f;
			}
			else
			{
				auto airaccel = g_CVar->FindVar("sv_airaccelerate");
				auto maxspeed = g_CVar->FindVar("sv_maxspeed");

				static int zhop = 0;
				double yawrad = Normalize_y(cmd->viewangles.yaw) * PI / 180;

				float speed = maxspeed->GetFloat();
				if (cmd->buttons & IN_DUCK)
					speed *= 0.333;

				double tau = g_GlobalVars->interval_per_tick, MA = speed * airaccel->GetFloat();

				int Sdir = 0, Fdir = 0;
				Vector velocity = g_LocalPlayer->m_vecVelocity();
				double vel[3] = { velocity[0], velocity[1], velocity[2] };
				double pos[2] = { 0, 0 };
				double dir[2] = { std::cos((cmd->viewangles[1] + 10 * zhop) * PI / 180), std::sin((cmd->viewangles[1] + 10 * zhop) * PI / 180) };
				cmd->viewangles.yaw = Normalize_y(yawrad * 180 / PI);
				Strafer.strafe_line_opt(yawrad, Sdir, Fdir, vel, pos, 30.0, tau, MA, pos, dir);
				cmd->sidemove = Sdir * 450;
			}
		}

	}
}