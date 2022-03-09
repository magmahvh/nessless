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
		static int ChokedPackets = -1;
		static bool yFlip;
		if (ChokedPackets < 0)
		{
			bSendPacket = true;
			ChokedPackets = 1;
		}
		else
		{
			bSendPacket = false;
			if (invert)
				yFlip ? pCmd->viewangles.yaw += 90.f : pCmd->viewangles.yaw -= 90.f;
			else
				yFlip ? pCmd->viewangles.yaw -= 90.f : pCmd->viewangles.yaw += 90.f;

			ChokedPackets = -1;
		}
		yFlip != yFlip;

		pCmd->viewangles.pitch = 89;
		pCmd->viewangles.yaw += 180;
	}
}