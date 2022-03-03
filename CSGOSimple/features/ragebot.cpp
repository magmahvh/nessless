#include "ragebot.hpp"
#include "autowall.hpp"

#include "..//helpers/math.hpp"
#include "..//helpers/input.hpp"

int wpnGroupRage(CHandle<C_BaseCombatWeapon> pWeapon) {

	if (!pWeapon)
		return -1;

	if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP)
		return 0;
	else if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SCAR20 || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_G3SG1)
		return 1;
	else if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08)
		return 2;
	else if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_DEAGLE || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		return 3;
	else if (pWeapon->IsPistol())
		return 4;
	else
		return 5;
}

bool Hitchance(float hitchance, C_BaseCombatWeapon* pWeapon)
{
	if (hitchance >= pWeapon->GetInaccuracy() * 1000) //1000 потому  что так надо =D Magma techonology
		return true;
	else return false;
}

float CRagebot::GetFovToPlayer(QAngle viewAngle, QAngle aimAngle)
{
	QAngle delta = aimAngle - viewAngle;
	Math::FixAngles(delta);
	return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
}

bool CRagebot::IsEnabled(CUserCmd* cmd)
{
	if (!g_Options.rage_enabled)
		return false;

	if (!g_EngineClient->IsInGame() || !g_LocalPlayer)
		return false;

	if (!g_LocalPlayer->IsAlive())
		return false;

	weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon || !weapon->IsGun())
		return false;

	if (!g_Options.ragebot[wpnGroupRage(weapon)].enabled)
		return false;

	if (!weapon->HasBullets())
		return false;

	return (cmd->buttons & IN_ATTACK) || (g_Options.ragebot[wpnGroupRage(weapon)].autoshot);
}

void CRagebot::Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle)
{
	Vector current, aim;

	Math::AngleVectors(currentAngle, current);
	Math::AngleVectors(aimAngle, aim);

	const Vector delta = aim - current;
	const Vector smoothed = current + delta;

	Math::VectorAngles(smoothed, angle);
}

void CRagebot::RCS(QAngle& angle, C_BasePlayer* target)
{
	static auto recoil_scale = g_CVar->FindVar("weapon_recoil_scale");
	auto scale = recoil_scale->GetFloat();

	QAngle punch = { 0, 0, 0 };

	if (target)
		punch = { current_punch.pitch * scale, current_punch.yaw * scale, 0 };

	if ((punch.pitch != 0.f || punch.yaw != 0.f) && current_punch.roll == 0.f) {
		angle -= punch;
		Math::FixAngles(angle);
	}
}

C_BasePlayer* CRagebot::GetClosestPlayer(CUserCmd* cmd, int& bestBone, float& bestFov, QAngle& bestAngles)
{
	target = nullptr;

	std::vector<int> hitboxes;

	if (g_Options.ragebot[wpnGroupRage(weapon)].hitboxes.head)
		hitboxes.emplace_back(HITBOX_HEAD);

	if (g_Options.ragebot[wpnGroupRage(weapon)].hitboxes.upper_chest)
	{
		hitboxes.emplace_back(HITBOX_UPPER_CHEST);
	}

	if (g_Options.ragebot[wpnGroupRage(weapon)].hitboxes.chest)
	{
		hitboxes.emplace_back(HITBOX_CHEST);
	}

	if (g_Options.ragebot[wpnGroupRage(weapon)].hitboxes.lower_chest)
	{
		hitboxes.emplace_back(HITBOX_LOWER_CHEST);
	}

	if (g_Options.ragebot[wpnGroupRage(weapon)].hitboxes.hands)
	{
		hitboxes.emplace_back(HITBOX_LEFT_FOREARM);
		hitboxes.emplace_back(HITBOX_LEFT_HAND);
		hitboxes.emplace_back(HITBOX_LEFT_UPPER_ARM);

		hitboxes.emplace_back(HITBOX_RIGHT_FOREARM);
		hitboxes.emplace_back(HITBOX_RIGHT_HAND);
		hitboxes.emplace_back(HITBOX_RIGHT_UPPER_ARM);
	}

	if (g_Options.ragebot[wpnGroupRage(weapon)].hitboxes.legs)
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

		if (!player->IsEnemy())
			continue;

		for (const auto hitbox : hitboxes)
		{
			Vector hitboxPos = player->GetHitboxPos(hitbox);
			QAngle ang;
			Math::VectorAngles(hitboxPos - eyePos, ang);
			const float fov = GetFovToPlayer(cmd->viewangles + last_punch * 2.f, ang);
			
			const auto damage = Autowall::Get().GetDamage(hitboxPos);

			if (!g_LocalPlayer->CanSeePlayer(player, hitboxPos))
			{
				if (!g_Options.ragebot[wpnGroupRage(weapon)].autowall)
					continue;
			}

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

void CRagebot::AutoStop(CUserCmd* cmd, CCSWeaponInfo* weapon_data) {
	if (g_Options.ragebot[wpnGroupRage(weapon)].autostop)
	{
		if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
			return;

		Vector velocity = g_LocalPlayer->m_vecVelocity();
		QAngle direction;

		Math::VectorAngles(velocity, direction);

		float speed = velocity.Length2D();

		direction.yaw = cmd->viewangles.yaw - direction.yaw;

		Vector forward;

		Math::AngleVectors(direction, forward);

		Vector source = forward * -speed;

		const float factor = max(source.x, source.y) / 450.f;
		source *= factor;

		if (speed > ((g_LocalPlayer->m_bIsScoped() ? weapon_data->flSpread : weapon_data->flSpreadAlt) * 0.37f))
		{
			cmd->forwardmove = source.x;
			cmd->sidemove = source.y;
		}
	}
}

void CRagebot::Run(CUserCmd* cmd)
{
	current_punch = g_LocalPlayer->m_aimPunchAngle();

	if (!IsEnabled(cmd))
	{
		last_punch = { 0, 0, 0 };
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

	auto angles = cmd->viewangles;
	const auto current = angles;

	float fov = FLT_MAX;

	int bestBone = -1;

	if (GetClosestPlayer(cmd, bestBone, fov, angles))
	{
		AutoStop(cmd, weapon_data);

		if (Hitchance(g_Options.ragebot[wpnGroupRage(weapon)].hitchance, weapon)) {

			if (g_Options.ragebot[wpnGroupRage(weapon)].autoshot)
				cmd->buttons |= IN_ATTACK;

		}

	}

	if ((cmd->buttons & IN_ATTACK))
		RCS(angles, target);

	last_punch = current_punch;

	if (!g_Options.ragebot[wpnGroupRage(weapon)].silent)
		Smooth(current, angles, angles);

	Math::FixAngles(angles);
	cmd->viewangles = angles;
	if (!g_Options.ragebot[wpnGroupRage(weapon)].silent)
		g_EngineClient->SetViewAngles(&angles);


	if (g_LocalPlayer->m_hActiveWeapon()->IsPistol())
	{
		const float server_time = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
		const float next_shot = g_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() - server_time;

		if (next_shot > 0)
			cmd->buttons &= ~IN_ATTACK;
	}
}