#include "antiaim.hpp"
#include "fakelag.hpp"
#include "../helpers/keybinds.hpp"\

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
		return 179;
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