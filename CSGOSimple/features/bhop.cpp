#include "bhop.hpp"
#include "../options.hpp"

#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
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

void BunnyHop::AutoStrafe(CUserCmd* cmd, QAngle va) 
	{
		if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
			return;

		if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
			return;

		static auto cl_sidespeed = g_CVar->FindVar("cl_sidespeed");
		auto side_speed = cl_sidespeed->GetFloat();

		if (g_Options.misc_autostrafe)
		{
			if (g_LocalPlayer->m_vecVelocity().Length2D() <= 5.0f)
			{
				C_BaseCombatWeapon* pCombatWeapon = g_LocalPlayer->m_hActiveWeapon().Get();
				if (pCombatWeapon)
				{
					if (pCombatWeapon->m_iItemDefinitionIndex() != WEAPON_SSG08)
					{
						if (!g_Options.misc_boostspeed)
							return;
					}
					else
						return;
				}
			}
	if (!g_Options.misc_wasdstrafes)
	{
		cmd->forwardmove = (10000.f / g_LocalPlayer->m_vecVelocity().Length2D() > 450.f) ? 450.f : 10000.f / g_LocalPlayer->m_vecVelocity().Length2D();
		cmd->sidemove = (cmd->mousedx != 0) ? (cmd->mousedx < 0.0f) ? -450.f : 450.f : (cmd->command_number & 1) ? -450.f : 450.f;

		return;
	}

	static bool bFlip = true;
	static float flOldYaw = cmd->viewangles.yaw;

	Vector vecVelocity = g_LocalPlayer->m_vecVelocity();
	vecVelocity.z = 0.0f;

	float_t flForwardMove = cmd->forwardmove;
	float_t flSideMove = cmd->sidemove;

	float flTurnVelocityModifier = bFlip ? 1.5f : -1.5f;
	QAngle angViewAngles = cmd->viewangles;

	if (flForwardMove || flSideMove)
	{
		cmd->forwardmove = 0.0f;
		cmd->sidemove = 0.0f;

		float m_flTurnAngle = atan2(-flSideMove, flForwardMove);
		angViewAngles.yaw += m_flTurnAngle * M_RADPI;
	}
	else if (flForwardMove)
		cmd->forwardmove = 0.0f;

	float flStrafeAngle = RAD2DEG(atan(15.0f / vecVelocity.Length2D()));
	if (flStrafeAngle > 90.0f)
		flStrafeAngle = 90.0f;
	else if (flStrafeAngle < 0.0f)
		flStrafeAngle = 0.0f;

	Vector vecTemp = Vector(0.0f, angViewAngles.yaw - flOldYaw, 0.0f);
	vecTemp.y = Math::NormalizeAngle(vecTemp.y);
	flOldYaw = angViewAngles.yaw;

	float flYawDelta = vecTemp.y;
	float flAbsYawDelta = fabs(flYawDelta);
	if (flAbsYawDelta <= flStrafeAngle || flAbsYawDelta >= 30.0f)
	{
		QAngle angVelocityAngle;
		Math::VectorAngles(vecVelocity, angVelocityAngle);

		vecTemp = Vector(0.0f, angViewAngles.yaw - angVelocityAngle.yaw, 0.0f);
		vecTemp.y = Math::NormalizeAngle(vecTemp.y);

		float flVelocityAngleYawDelta = vecTemp.y;
		float flVelocityDegree = Math::clamp(RAD2DEG(atan(30.0f / vecVelocity.Length2D())), 0.0f, 90.0f) * 0.01f;

		if (flVelocityAngleYawDelta <= flVelocityDegree || vecVelocity.Length2D() <= 15.0f)
		{
			if (-flVelocityDegree <= flVelocityAngleYawDelta || vecVelocity.Length2D() <= 15.0f)
			{
				angViewAngles.yaw += flStrafeAngle * flTurnVelocityModifier;
				cmd->sidemove = 450.0f * flTurnVelocityModifier;
			}
			else
			{
				angViewAngles.yaw = angVelocityAngle.yaw - flVelocityDegree;
				cmd->sidemove = 450.0f;
			}
		}
		else
		{
			angViewAngles.yaw = angVelocityAngle.yaw + flVelocityDegree;
			cmd->sidemove = -450.0f;
		}
	}
	else if (flYawDelta > 0.0f)
		cmd->sidemove = 450.0f;
	else if (flYawDelta < 0.0f)
		cmd->sidemove = 450.0f;

	Vector vecMove = Vector(cmd->forwardmove, cmd->sidemove, 0.0f);
	float flSpeed = vecMove.Length();

	QAngle angMoveAngle;
	Math::VectorAngles(vecMove, angMoveAngle);

	float flNormalizedX = fmod(cmd->viewangles.pitch + 180.0f, 360.0f) - 180.0f;
	float flNormalizedY = fmod(cmd->viewangles.yaw + 180.0f, 360.0f) - 180.0f;
	float flYaw = DEG2RAD((flNormalizedY - angViewAngles.yaw) + angMoveAngle.yaw);

	if (cmd->viewangles.pitch <= 200.0f && (flNormalizedX >= 90.0f || flNormalizedX <= -90.0f || (cmd->viewangles.pitch >= 90.0f && cmd->viewangles.pitch <= 200.0f) || cmd->viewangles.pitch <= -90.0f))
		cmd->forwardmove = -cos(flYaw) * flSpeed;
	else
		cmd->forwardmove = cos(flYaw) * flSpeed;

	cmd->sidemove = sin(flYaw) * flSpeed;

	bFlip = !bFlip;
	}
}


#if 0
	static bool leftRight;

	bool inMove = cmd->buttons & IN_BACK || cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT;

	if (cmd->buttons & IN_FORWARD && g_LocalPlayer->m_vecVelocity().Length() <= 50.0f)
		cmd->forwardmove = 250.0f;

	float yaw_change = 0.0f;

	if (g_LocalPlayer->m_vecVelocity().Length() > 50.f)
		yaw_change = 30.0f * fabsf(30.0f / g_LocalPlayer->m_vecVelocity().Length());

	C_BaseCombatWeapon* ActiveWeapon = g_LocalPlayer->m_hActiveWeapon();

	if (ActiveWeapon && ActiveWeapon->CanFire() && cmd->buttons & IN_ATTACK)
		yaw_change = 0.0f;

	QAngle viewAngles = va;

	bool OnGround = (g_LocalPlayer->m_fFlags() & FL_ONGROUND);
	if (!OnGround && !inMove) {
		if (leftRight || cmd->mousedx > 1) {
			viewAngles.yaw += yaw_change;
			cmd->sidemove = 350.0f;
		}
		else if (!leftRight || cmd->mousedx < 1) {
			viewAngles.yaw -= yaw_change;
			cmd->sidemove = -350.0f;
		}

		leftRight = !leftRight;
	}
	viewAngles.Normalize();
	Math::ClampAngles(viewAngles);
#endif
	//MovementFix::Get().Correct(viewAngles, cmd, cmd->forwardmove, cmd->sidemove);
