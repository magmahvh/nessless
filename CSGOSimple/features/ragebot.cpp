#include "ragebot.hpp"
#include "autowall.hpp"

#include "../helpers/math.hpp"
#include "../helpers/input.hpp"
#include "../helpers/logs.hpp"
#include "lagcompensation.hpp"
#include "resolver.hpp"
#include "../render.hpp"
#include <algorithm>

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

float hitchance(IClientEntity* pLocal, C_BaseCombatWeapon* pWeapon)
{
	float hitchance = 101;
	if (!pWeapon) return 0;
	if (g_Options.ragebot[wpnGroupRage(pWeapon)].hitchance > 1)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;

	}
	return hitchance;
}

float CRagebot::GetBodyScale(C_BasePlayer* player)
{
	if (!(player->m_fFlags() & FL_ONGROUND))
		return 0.f;

	if (g_Options.ragebot[wpnGroupRage(weapon)].multipoint_body > 0)
		return std::clamp(g_Options.ragebot[wpnGroupRage(weapon)].multipoint_body / 100.f, 0.f, 0.75f);

	auto factor = [](float x, float min, float max) {
		return 1.f - 1.f / (1.f + pow(2.f, (-([](float x, float min, float max) {
			return ((x - min) * 2.f) / (max - min) - 1.f;
			}(x, min, max)) / 0.115f)));
	}(player->m_angAbsOrigin().DistTo(g_LocalPlayer->GetEyePos()), 0.f, weapon->GetCSWeaponData()->flRange / 4.f);

	if (weapon->IsSniper() && !weapon->scopeLevel())
		factor = 0.f;

	if (g_LocalPlayer->m_flDuckAmount() >= 0.9f /*&& !fakeduck*/)
		return 0.65f;

	return std::clamp(factor, 0.f, 0.75f);
}

float CRagebot::GetHeadScale(C_BasePlayer* player)
{
	if (g_Options.ragebot[wpnGroupRage(weapon)].multipoint_head > 0)
		return std::clamp(g_Options.ragebot[wpnGroupRage(weapon)].multipoint_head / 100.f, 0.f, 0.80f);

	/*if (!vars.misc.antiuntrusted)
		return 0.90f;*/

	/*if (g_Binds[bind_fake_duck].active)
		return 0.70f;*/

	if (player->m_fFlags() & FL_ONGROUND)
		return GetBodyScale(player);
	else
		return 0.75f;
}

std::vector<std::pair<Vector, bool>> CRagebot::GetMultipoints(C_BasePlayer* pBaseEntity, int iHitbox, matrix3x4_t bones[128]) {
	std::vector<std::pair<Vector, bool>> points;

	const model_t* model = pBaseEntity->GetModel();
	if (!model)
		return points;

	studiohdr_t* hdr = g_MdlInfo->GetStudiomodel(model);
	if (!hdr)
		return points;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(pBaseEntity->m_nHitboxSet());
	if (!set)
		return points;

	mstudiobbox_t* bbox = set->GetHitbox(iHitbox);
	if (!bbox)
		return points;

	/*Vector max = bbox->bbmax;
	Vector min = bbox->bbmin;*/
	Vector center = pBaseEntity->GetHitboxPos(iHitbox);//(bbox->bbmin + bbox->bbmax) / 2.f;

	if (iHitbox == HITBOX_HEAD) {
		float r = bbox->m_flRadius * GetHeadScale(pBaseEntity);
		points.emplace_back(center, true);

		constexpr float rotation = 0.70710678f;
		points.emplace_back(Vector{ center.x + (rotation * r), center.y + (-rotation * r), center.z }, false);

		Vector right{ center.x, center.y, center.z + r };
		points.emplace_back(right, false);

		Vector left{ center.x, center.y, center.z - r };
		points.emplace_back(left, false);
		
		Vector back{ center.x, center.y - r, center.z };
		points.emplace_back(back, false);

		CCSGOPlayerAnimState* state = pBaseEntity->GetPlayerAnimState();
		if (state && pBaseEntity->m_vecVelocity().Length() <= 0.1f && pBaseEntity->m_angEyeAngles().pitch <= 75.f) {
			points.emplace_back(Vector{ center.x - r, center.y, center.z }, false);
		}
	}
	else {
		float r = bbox->m_flRadius * GetBodyScale(pBaseEntity);
		if (iHitbox == HITBOX_STOMACH) {
			// center.
			points.emplace_back(center, true);
			points.emplace_back(Vector(center.x, center.y, center.z + r), false);
			points.emplace_back(Vector(center.x, center.y, center.z - r), false);
			// back.
			points.emplace_back(Vector{ center.x, center.y - r, center.z }, true);
		}

		else if (iHitbox == HITBOX_PELVIS || iHitbox == HITBOX_UPPER_CHEST) {
			//points.emplace_back(center);
			// left & right points
			points.emplace_back(Vector(center.x, center.y, center.z + r), false);
			points.emplace_back(Vector(center.x, center.y, center.z - r), false);
		}

		// other stomach/chest hitboxes have 2 points.
		else if (iHitbox == HITBOX_CHEST || iHitbox == HITBOX_LOWER_CHEST) {
			// left & right points
			points.emplace_back(Vector(center.x, center.y, center.z + r), false);
			points.emplace_back(Vector(center.x, center.y, center.z - r), false);
			// add extra point on back.
			points.emplace_back(Vector{ center.x, center.y - r, center.z }, false);
		}

		else if (iHitbox == HITBOX_RIGHT_CALF || iHitbox == HITBOX_LEFT_CALF) {
			// add center.
			points.emplace_back(center, true);

			// half bottom.
			points.emplace_back(Vector{ center.x - (bbox->m_flRadius / 2.f), center.y, center.z }, false);
		}

		else if (iHitbox == HITBOX_RIGHT_THIGH || iHitbox == HITBOX_LEFT_THIGH) {
			// add center.
			points.emplace_back(center, true);
		}

		// arms get only one point.
		else if (iHitbox == HITBOX_RIGHT_UPPER_ARM || iHitbox == HITBOX_LEFT_UPPER_ARM) {
			// elbow.
			points.emplace_back(Vector{ center.x + bbox->m_flRadius, center.y, center.z }, false);
		}
		else
			points.emplace_back(center, true);
	}

	return points;

	/*for (auto& p : points)
		Math::VectorTransform(p.first, bones[bbox->bone], p.first);*/
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

int CRagebot::GetMinimumDamage(C_BasePlayer* target) {
	int damage = g_Options.ragebot[wpnGroupRage(weapon)].damage;

	if (damage <= 100) return damage;
	else {
		int hpp_dmg = damage - 100;
		return target->m_iHealth() + hpp_dmg;
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

	hitboxes_active = hitboxes;

	const Vector eyePos = g_LocalPlayer->GetEyePos();

	float damage;

	for (auto i = 1; i <= g_GlobalVars->maxClients; i++)
	{
		C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(i);

		if (!player || !player->IsAlive() || !player->IsPlayer() || player->m_bGunGameImmunity())
			continue;

		if (!player->IsEnemy())
			continue;

		//cmd->tick_count = Math::TIME_TO_TICKS(player->m_flSimulationTime() + CLagCompensation::Get().LagFix());

		for (const auto hitbox : hitboxes)
		{
			for (auto hitboxPos : GetMultipoints(player, hitbox, nullptr)) {
				Vector hbPos = hitboxPos.first;
				QAngle ang;
				Math::VectorAngles(hbPos - eyePos, ang);
				const float fov = GetFovToPlayer(cmd->viewangles + last_punch * 2.f, ang);

				damage = CAutoWall::Get().CanHit(hbPos);

				if (damage < CRagebot::GetMinimumDamage(player)) continue;

				if (!g_LocalPlayer->CanSeePlayer(player, hbPos))
				{
					if (!g_Options.ragebot[wpnGroupRage(weapon)].autowall)
						continue;
				}

				AutoStop(cmd, weapon->GetCSWeaponData());

				if (bestFov > fov)
				{
					bestBone = hitbox;
					bestAngles = ang;
					bestFov = fov;
					target = player;
				}
			}
			//Vector hitboxPos = player->GetHitboxPos(hitbox);
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

void Attack(CUserCmd* cmd, C_BasePlayer* target, int bone) {
	cmd->buttons |= IN_ATTACK;
	std::stringstream log;
	log << "Shot fired at ";
	log << target->m_iName().c_str();
	log << "; dmg: ";
	log << (CAutoWall::Get().CanHit(target->GetHitboxPos(bone)));
	Logs::Get().Create(log.str());
	g_Ragebot->shots_fired[target->EntIndex()]++;
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

	auto weapon_canfire = weapon->CanFire();
	if (!weapon_canfire)
		return;

	auto angles = cmd->viewangles;
	const auto current = angles;

	float fov = FLT_MAX;

	int bestBone = -1;

	if (GetClosestPlayer(cmd, bestBone, fov, angles))
	{

		if (g_Options.ragebot[wpnGroupRage(weapon)].hitchance < hitchance(g_LocalPlayer, weapon)) {

			if (g_Options.ragebot[wpnGroupRage(weapon)].autoshot)
				Attack(cmd, target, bestBone);

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
			Attack(cmd, target, bestBone);
	}
}

void CRagebot::Slowwalk(CUserCmd* pCmd) {
	if (!g_Options.slowwalk || !g_InputSys->IsKeyDown(g_Options.slowwalkkey))
		return;

	auto weapon_handle = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon_handle)
		return;

	if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
		return;

	float amount = 0.0034f * g_Options.slowwalkspeed;

	Vector velocity = g_LocalPlayer->m_vecVelocity();
	QAngle direction;

	Math::VectorAngles(velocity, direction);

	float speed = velocity.Length2D();

	direction.yaw = pCmd->viewangles.yaw - direction.yaw;

	Vector forward;

	Math::AngleVectors(direction, forward);

	Vector source = forward * -speed;

	if (speed >= (weapon_handle->GetCSWeaponData()->flMaxPlayerSpeed * amount))
	{
		pCmd->forwardmove = source.x;
		pCmd->sidemove = source.y;
	}
}