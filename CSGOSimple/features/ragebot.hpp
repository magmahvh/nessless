#pragma once

#include "..//options.hpp"
#include "..//valve_sdk/csgostructs.hpp"

class CRagebot
{
public:
	CRagebot()
	{
		current_punch = last_punch = { 0, 0, 0 };
		target = nullptr;
	}

	void Run(CUserCmd* cmd);
	bool IsEnabled(CUserCmd* cmd);
	float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle);

	CHandle<C_BaseCombatWeapon> weapon;

private:
	void RCS(QAngle& angle, C_BasePlayer* target);
	void Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle);
	C_BasePlayer* GetClosestPlayer(CUserCmd* cmd, int& bestBone, float& fov, QAngle& angles);

	C_BasePlayer* target = nullptr;

	QAngle current_punch = { 0, 0, 0 };
	QAngle last_punch = { 0, 0, 0 };
};

inline CRagebot* g_Ragebot;