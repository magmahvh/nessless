#include "aimbot.hpp"
#include "autowall.hpp"

#include "..//helpers/math.hpp"
#include "..//helpers/input.hpp"

int wpnGroupLegit(CHandle<C_BaseCombatWeapon> pWeapon) {

	if (!pWeapon)
		return -1;

	if (pWeapon->IsPistol() && pWeapon->m_Item().m_iItemDefinitionIndex() != WEAPON_DEAGLE)
		return 0;
	else if (pWeapon->IsRifle())
		return 1;
	else if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_DEAGLE)
		return 2;
	else if (pWeapon->IsSniper())
		return 3;
	else
		return 4;
}

float CLegitbot::GetFovToPlayer(QAngle viewAngle, QAngle aimAngle)
{
	QAngle delta = aimAngle - viewAngle;
	Math::FixAngles(delta);
	return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
}

bool CLegitbot::IsLineGoesThroughSmoke(Vector startPos, Vector endPos)
{
	static auto LineGoesThroughSmokeFn = (bool(*)(Vector, Vector))Utils::PatternScan2("client.dll", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
	return LineGoesThroughSmokeFn(startPos, endPos);
}

bool CLegitbot::IsEnabled(CUserCmd* cmd)
{
	if (!g_Options.legit_enabled)
		return false;

	if (!g_EngineClient->IsInGame() || !g_LocalPlayer)
		return false;

	if (!g_LocalPlayer->IsAlive())
		return false;

	weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon || !weapon->IsGun())
		return false;

	if (!g_Options.legitbot[wpnGroupLegit(weapon)].enabled)
		return false;

	if (!weapon->HasBullets())
		return false;

	return (cmd->buttons & IN_ATTACK) || (g_Options.legitbot[wpnGroupLegit(weapon)].autofire.enabled && GetAsyncKeyState(g_Options.legitbot[wpnGroupLegit(weapon)].autofire.hotkey));
}

void CLegitbot::Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle)
{
	auto smooth_value = max(1.0f, g_Options.legitbot[wpnGroupLegit(weapon)].smooth);
	
	Vector current, aim;

	Math::AngleVectors(currentAngle, current);
	Math::AngleVectors(aimAngle, aim);

	const Vector delta = aim - current;
	const Vector smoothed = current + delta / smooth_value;

	Math::VectorAngles(smoothed, angle);
}

void CLegitbot::RCS(QAngle& angle, C_BasePlayer* target)
{
	if (!g_Options.legitbot[wpnGroupLegit(weapon)].rcs.enabled || shotsFired < g_Options.legitbot[wpnGroupLegit(weapon)].rcs.start - 1)
		return;

	if (!g_Options.legitbot[wpnGroupLegit(weapon)].rcs.x && !g_Options.legitbot[wpnGroupLegit(weapon)].rcs.y)
		return;

	static auto recoil_scale = g_CVar->FindVar("weapon_recoil_scale");
	auto scale = recoil_scale->GetFloat();

	const auto x = float(g_Options.legitbot[wpnGroupLegit(weapon)].rcs.y) / 100.f * scale;
	const auto y = float(g_Options.legitbot[wpnGroupLegit(weapon)].rcs.x) / 100.f * scale;

	QAngle punch = { 0, 0, 0 };

	if (target)
		punch = { current_punch.pitch * x, current_punch.yaw * y, 0 };

	if ((punch.pitch != 0.f || punch.yaw != 0.f) && current_punch.roll == 0.f) {
		angle -= punch;
		Math::FixAngles(angle);
	}
}

float CLegitbot::GetFov()
{
	if (g_Options.legitbot[wpnGroupLegit(weapon)].silent)
		return g_Options.legitbot[wpnGroupLegit(weapon)].silent_fov;

	return g_Options.legitbot[wpnGroupLegit(weapon)].fov;
}

C_BasePlayer* CLegitbot::GetClosestPlayer(CUserCmd* cmd, int& bestBone, float& bestFov, QAngle& bestAngles)
{
	if (target && !kill_delay && g_Options.legitbot[wpnGroupLegit(weapon)].kill_delay > 0 && !target->IsAlive())
	{
		target = nullptr;
		kill_delay = true;
		kill_delay_time = int(GetTickCount()) + g_Options.legitbot[wpnGroupLegit(weapon)].kill_delay;
	}
	if (kill_delay)
	{
		if (kill_delay_time <= int(GetTickCount()))
			kill_delay = false;
		else
			return nullptr;
	}

	target = nullptr;

	std::vector<int> hitboxes;

	if (g_Options.legitbot[wpnGroupLegit(weapon)].hitboxes.head)
		hitboxes.emplace_back(HITBOX_HEAD);

	if (g_Options.legitbot[wpnGroupLegit(weapon)].hitboxes.chest)
	{
		hitboxes.emplace_back(HITBOX_UPPER_CHEST);
		hitboxes.emplace_back(HITBOX_CHEST);
		hitboxes.emplace_back(HITBOX_LOWER_CHEST);
	}

	if (g_Options.legitbot[wpnGroupLegit(weapon)].hitboxes.hands)
	{
		hitboxes.emplace_back(HITBOX_LEFT_FOREARM);
		hitboxes.emplace_back(HITBOX_LEFT_HAND);
		hitboxes.emplace_back(HITBOX_LEFT_UPPER_ARM);

		hitboxes.emplace_back(HITBOX_RIGHT_FOREARM);
		hitboxes.emplace_back(HITBOX_RIGHT_HAND);
		hitboxes.emplace_back(HITBOX_RIGHT_UPPER_ARM);
	}

	if (g_Options.legitbot[wpnGroupLegit(weapon)].hitboxes.legs)
	{
		hitboxes.emplace_back(HITBOX_LEFT_FOOT);
		hitboxes.emplace_back(HITBOX_LEFT_CALF);
		hitboxes.emplace_back(HITBOX_LEFT_THIGH);

		hitboxes.emplace_back(HITBOX_RIGHT_FOOT);
		hitboxes.emplace_back(HITBOX_LEFT_CALF);
		hitboxes.emplace_back(HITBOX_LEFT_THIGH);
	}
	
	const Vector eyePos = g_LocalPlayer->GetEyePos();

	for (auto i = 1; i <= g_GlobalVars->maxClients; i++)
	{
		C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(i);

		if (!player || !player->IsAlive() || !player->IsPlayer() || player->m_bGunGameImmunity())
			continue;

		if (!player->IsEnemy() && !g_Options.legitbot[wpnGroupLegit(weapon)].deathmatch)
			continue;

		for (const auto hitbox : hitboxes)
		{
			Vector hitboxPos = player->GetHitboxPos(hitbox);
			QAngle ang;
			Math::VectorAngles(hitboxPos - eyePos, ang);
			const float fov = GetFovToPlayer(cmd->viewangles + last_punch * 2.f, ang);

			if (fov > GetFov())
				continue;

			if (!g_LocalPlayer->CanSeePlayer(player, hitboxPos))
			{
				if (!g_Options.legitbot[wpnGroupLegit(weapon)].autowall.enabled)
					continue;

				//const auto damage = c_autowall::get()->autowall(g_LocalPlayer->GetEyePos(), hitboxPos, g_LocalPlayer, player).damage;
				auto damage = 0.f;
				C_AutoWall::Get().PenetrateWall(g_LocalPlayer, g_LocalPlayer->m_hActiveWeapon(), hitboxPos, damage);

				if (damage < g_Options.legitbot[wpnGroupLegit(weapon)].autowall.min_damage)
					continue;
			}

			if (g_Options.legitbot[wpnGroupLegit(weapon)].smoke_check && IsLineGoesThroughSmoke(eyePos, hitboxPos))
				continue;

			if (bestFov > fov)
			{
				bestBone = hitbox;
				bestAngles = ang;
				bestFov = fov;
				target = player;
			}
		}
	}

	return target;
}

void CLegitbot::Run(CUserCmd* cmd)
{
	if (int(GetTickCount()) > lastShotTick + 50)
		shotsFired = 0;

	current_punch = g_LocalPlayer->m_aimPunchAngle();

	if (!IsEnabled(cmd))
	{
		last_punch = { 0, 0, 0 };
		shot_delay = false;
		kill_delay = false;
		target = nullptr;
		return;
	}

	//RandomSeed(cmd->command_number);

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	auto weapon_data = weapon->GetCSWeaponData();
	if (!weapon_data)
		return;

	auto weapon_canfire = weapon->CanFire();
	if (!weapon_canfire)
		return;

	if (g_Options.legitbot[wpnGroupLegit(weapon)].flash_check && g_LocalPlayer->IsFlashed())
		return;

	auto angles = cmd->viewangles;
	const auto current = angles;

	float fov = FLT_MAX;

	int bestBone = -1;

	if (GetClosestPlayer(cmd, bestBone, fov, angles))
	{
		if (!g_Options.legitbot[wpnGroupLegit(weapon)].silent && !shot_delay && g_Options.legitbot[wpnGroupLegit(weapon)].shot_delay > 0 && !shotsFired)
		{
			shot_delay = true;
			shot_delay_time = int(GetTickCount()) + g_Options.legitbot[wpnGroupLegit(weapon)].shot_delay;
		}

		if (shot_delay && shot_delay_time <= int(GetTickCount()))
			shot_delay = false;

		if (shot_delay)
			cmd->buttons &= ~IN_ATTACK;

		if (g_Options.legitbot[wpnGroupLegit(weapon)].autofire.enabled && GetAsyncKeyState(g_Options.legitbot[wpnGroupLegit(weapon)].autofire.hotkey))
			cmd->buttons |= IN_ATTACK;
	}

	if ((cmd->buttons & IN_ATTACK) /*&& !IsSilent()*/)
		RCS(angles, target);

	last_punch = current_punch;

	if (!g_Options.legitbot[wpnGroupLegit(weapon)].silent)
		Smooth(current, angles, angles);

	Math::FixAngles(angles);
	cmd->viewangles = angles;
	if (!g_Options.legitbot[wpnGroupLegit(weapon)].silent)
		g_EngineClient->SetViewAngles(&angles);


	if (g_LocalPlayer->m_hActiveWeapon()->IsPistol() && g_Options.legitbot[wpnGroupLegit(weapon)].autopistol)
	{
		const float server_time = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
		const float next_shot = g_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() - server_time;

		if (next_shot > 0)
			cmd->buttons &= ~IN_ATTACK;
	}

	if (cmd->buttons & IN_ATTACK)
	{
		lastShotTick = GetTickCount();
		shotsFired++;
	}
}