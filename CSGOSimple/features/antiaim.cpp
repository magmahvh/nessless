#include "antiaim.hpp"

static bool invert = false;
void Antiaim::Run(CUserCmd* pCmd, bool& bSendPacket)
{
	if (!g_Options.antiaim)
		return;

	if ((pCmd->buttons & IN_USE) || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER)
		return;

	//for the memes
	QAngle oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;
	if (GetAsyncKeyState(g_Options.antiaim_flip))
		invert = !invert;

	if ((!pCmd->buttons & IN_ATTACK))
	{
		if (!bSendPacket) {
			pCmd->viewangles.yaw += invert ? 59 : -59;
			if (std::abs(pCmd->sidemove) < 5.0f) {
				if (pCmd->buttons & IN_DUCK) 
					pCmd->sidemove = pCmd->tick_count & 1 ? 3.25f : -3.25f;
				else
					pCmd->sidemove = pCmd->tick_count & 1 ? 1.1f : -1.1f;
			}
		}

		pCmd->viewangles.pitch = 89;
		pCmd->viewangles.yaw += 180;

	}
}