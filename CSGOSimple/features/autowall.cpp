#include "autowall.hpp"
#include "Math.h"
#include "../valve_sdk/interfaces/IEngineTrace.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../options.hpp"
#define HITGROUP_GENERIC 0
#define HITGROUP_HEAD 1
#define HITGROUP_CHEST 2
#define HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7
#define HITGROUP_GEAR 10

inline bool CGameTrace::DidHitWorld() const
{
	return hit_entity->EntIndex() == 0;
}

inline bool CGameTrace::DidHitNonWorldEntity() const
{
	return hit_entity != NULL && !DidHitWorld();
}

void CAutoWall::TraceLine(Vector& absStart, Vector& absEnd, unsigned int mask, IClientEntity* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(absStart, absEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	g_EngineTrace->TraceRay(ray, mask, &filter, ptr);
}

void CAutoWall::ClipTraceToPlayers(const Vector& absStart, const Vector absEnd, unsigned int mask, ITraceFilter* filter, CGameTrace* tr)
{
	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();

	static DWORD dwAddress = (DWORD)Utils::PatternScan(GetModuleHandleA("client.dll"), "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10");

	if (!dwAddress)
		return;

	_asm
	{
		MOV		EAX, filter
		LEA		ECX, tr
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, absEnd
		LEA		ECX, absStart
		CALL	dwAddress
		ADD		ESP, 0xC
	}
}

void CAutoWall::GetBulletTypeParameters(float& maxRange, float& maxDistance, char* bulletType, bool sv_penetration_type)
{
	if (sv_penetration_type)
	{
		maxRange = 35.0;
		maxDistance = 3000.0;
	}
	else
	{
		//Play tribune to framerate. Thanks, stringcompare
		//Regardless I doubt anyone will use the old penetration system anyway; so it won't matter much.
		if (!strcmp(bulletType, ("BULLET_PLAYER_338MAG")))
		{
			maxRange = 45.0;
			maxDistance = 8000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_762MM")))
		{
			maxRange = 39.0;
			maxDistance = 5000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_556MM")) || !strcmp(bulletType, ("BULLET_PLAYER_556MM_SMALL")) || !strcmp(bulletType, ("BULLET_PLAYER_556MM_BOX")))
		{
			maxRange = 35.0;
			maxDistance = 4000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_57MM")))
		{
			maxRange = 30.0;
			maxDistance = 2000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_50AE")))
		{
			maxRange = 30.0;
			maxDistance = 1000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_357SIG")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_SMALL")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_P250")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_MIN")))
		{
			maxRange = 25.0;
			maxDistance = 800.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_9MM")))
		{
			maxRange = 21.0;
			maxDistance = 800.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_45ACP")))
		{
			maxRange = 15.0;
			maxDistance = 500.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_BUCKSHOT")))
		{
			maxRange = 0.0;
			maxDistance = 0.0;
		}
	}
}

bool CAutoWall::BreakableEntity(IClientEntity* entity)
{
	ClientClass* pClass = (ClientClass*)entity->GetClientClass();

	if (!pClass)
		return false;

	if (pClass == nullptr)
		return false;

	return pClass->m_ClassID == ClassId_CBreakableProp || pClass->m_ClassID == ClassId_CBreakableSurface;
}

void CAutoWall::ScaleDamage(CGameTrace& enterTrace, CCSWeaponInfo* weaponData, float& currentDamage)
{
	if (!g_LocalPlayer)
		return;

	C_BaseCombatWeapon* weapon = (C_BaseCombatWeapon*)g_LocalPlayer->m_hActiveWeapon();

	bool hasHeavyArmor = static_cast<C_BasePlayer*>(enterTrace.hit_entity)->m_bHasHeavyArmor();
	int armorValue = static_cast<C_BasePlayer*>(enterTrace.hit_entity)->m_ArmorValue();

	//screw making a new function, lambda beste. ~ Does the person have armor on for the hitbox checked?
	auto IsArmored = [&enterTrace]() -> bool
	{
		switch (enterTrace.hitgroup)
		{
		case HITGROUP_HEAD:
			return static_cast<C_BasePlayer*>(enterTrace.hit_entity)->m_bHasHelmet(); //screw compiler errors - force-convert it to a bool via (!!)
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	};

	switch (enterTrace.hitgroup)
	{
	case HITGROUP_HEAD:
		currentDamage *= hasHeavyArmor ? 2.f : 4.f; //Heavy Armor does 1/2 damage
		break;
	case HITGROUP_STOMACH:
		currentDamage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		currentDamage *= 0.75f;
		break;
	default:
		break;
	}

	if (armorValue > 0 && IsArmored())
	{
		float bonusValue = 1.f, armorBonusRatio = 0.5f, armorRatio = weaponData->flArmorRatio / 2.f;

		//Damage gets modified for heavy armor users
		if (hasHeavyArmor)
		{
			armorBonusRatio = 0.33f;
			armorRatio *= 0.5f;
			bonusValue = 0.33f;
		}

		auto NewDamage = currentDamage * armorRatio;

		if (hasHeavyArmor)
			NewDamage *= 0.85f;

		if (((currentDamage - (currentDamage * armorRatio)) * (bonusValue * armorBonusRatio)) > armorValue)
			NewDamage = currentDamage - (armorValue / armorBonusRatio);

		currentDamage = NewDamage;
	}
}

bool CAutoWall::TraceToExit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction)
{
	Vector start, end;
	float maxDistance = 90.f, rayExtension = 4.f, currentDistance = 0;
	int firstContents = 0;

	while (currentDistance <= maxDistance)
	{
		currentDistance += rayExtension;

		start = startPosition + direction * currentDistance;

		if (!firstContents)
			firstContents = g_EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		int pointContents = g_EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		if (!(pointContents & MASK_SHOT_HULL) || pointContents & CONTENTS_HITBOX && pointContents != firstContents)
		{
			end = start - (direction * rayExtension);

			TraceLine(start, end, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr, &exitTrace);

			if (exitTrace.startsolid && exitTrace.surface.flags & SURF_HITBOX)
			{
				TraceLine(start, startPosition, MASK_SHOT_HULL, exitTrace.hit_entity, &exitTrace);

				if (exitTrace.DidHit() && !exitTrace.startsolid)
				{
					start = exitTrace.endpos;
					return true;
				}
				continue;
			}

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{

				if (BreakableEntity(enterTrace.hit_entity) && BreakableEntity(exitTrace.hit_entity))
					return true;

				if (enterTrace.surface.flags & SURF_NODRAW || !(exitTrace.surface.flags & SURF_NODRAW) && (exitTrace.plane.normal.Dot(direction) <= 1.f))
				{
					float multAmount = exitTrace.fraction * 4.f;
					start -= direction * multAmount;
					return true;
				}

				continue;
			}

			if (!exitTrace.DidHit() || exitTrace.startsolid)
			{
				if (enterTrace.DidHitNonWorldEntity() && BreakableEntity(enterTrace.hit_entity))
				{
					exitTrace = enterTrace;
					exitTrace.endpos = start + direction;
					return true;
				}

				continue;
			}
		}
	}

	return false;
}

bool CAutoWall::HandleBulletPenetration(CCSWeaponInfo* weaponData, CGameTrace& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration)
{
	//Because there's been issues regarding this- putting this here.
	if (&currentDamage == nullptr)
	{
		return false;
	}

	auto data = FireBulletData(g_LocalPlayer->GetEyePos());
	data.filter = CTraceFilter();
	data.filter.pSkip = g_LocalPlayer;
	CGameTrace exitTrace;
	C_BasePlayer* pEnemy = (C_BasePlayer*)enterTrace.hit_entity;

	surfacedata_t* enterSurfaceData = g_PhysSurface->GetSurfaceData(enterTrace.surface.surfaceProps);
	int enterMaterial = enterSurfaceData->game.material;

	float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
	float enterDamageModifier = enterSurfaceData->game.flDamageModifier;
	float thickness, modifier, lostDamage, finalDamageModifier, combinedPenetrationModifier;
	bool isSolidSurf = ((enterTrace.contents >> 3) & CONTENTS_SOLID);
	bool isLightSurf = ((enterTrace.surface.flags >> 7) & SURF_LIGHT);

	if (possibleHitsRemaining <= 0
		|| (enterTrace.surface.name == (const char*)0x2227c261 && exitTrace.surface.name == (const char*)0x2227c868)
		|| (!possibleHitsRemaining && !isLightSurf && !isSolidSurf && enterMaterial != CHAR_TEX_GRATE && enterMaterial != CHAR_TEX_GLASS)
		|| weaponData->flPenetration <= 0.f
		|| !TraceToExit(enterTrace, exitTrace, enterTrace.endpos, direction)
		&& !(g_EngineTrace->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL, nullptr) & MASK_SHOT_HULL))
	{
		return false;
	}

	surfacedata_t* exitSurfaceData = g_PhysSurface->GetSurfaceData(exitTrace.surface.surfaceProps);
	int exitMaterial = exitSurfaceData->game.material;
	float exitSurfPenetrationModifier = exitSurfaceData->game.flPenetrationModifier;
	float exitDamageModifier = exitSurfaceData->game.flDamageModifier;

	if (sv_penetration_type)
	{
		if (enterMaterial == CHAR_TEX_GRATE || enterMaterial == CHAR_TEX_GLASS)
		{
			combinedPenetrationModifier = 3.f;
			finalDamageModifier = 0.05f;
		}
		else if (isSolidSurf || isLightSurf)
		{
			combinedPenetrationModifier = 1.f;
			finalDamageModifier = 0.16f;
		}
		else if (enterMaterial == CHAR_TEX_FLESH && (g_LocalPlayer->m_iTeamNum() == pEnemy->m_iTeamNum() && ff_damage_reduction_bullets == 0.f))
		{
			if (ff_damage_bullet_penetration == 0.f)
			{
				return false;
			}
			combinedPenetrationModifier = ff_damage_bullet_penetration;
			finalDamageModifier = 0.16f;
		}
		else
		{
			combinedPenetrationModifier = (enterSurfPenetrationModifier + exitSurfPenetrationModifier) / 2.f;
			finalDamageModifier = 0.16f;
		}

		if (enterMaterial == exitMaterial)
		{
			if (exitMaterial == CHAR_TEX_CARDBOARD || exitMaterial == CHAR_TEX_WOOD)
				combinedPenetrationModifier = 3.f;
			else if (exitMaterial == CHAR_TEX_PLASTIC)
				combinedPenetrationModifier = 2.f;
		}

		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr();
		modifier = fmaxf(1.f / combinedPenetrationModifier, 0.f);

		lostDamage = fmaxf(
			((modifier * thickness) / 24.f)
			+ ((currentDamage * finalDamageModifier)
				+ (fmaxf(3.75f / penetrationPower, 0.f) * 3.f * modifier)), 0.f);

		if (lostDamage > currentDamage)
		{
			return false;
		}

		if (lostDamage > 0.f)
			currentDamage -= lostDamage;

		if (currentDamage < 1.f)
		{
			return false;
		}

		eyePosition = exitTrace.endpos;
		--possibleHitsRemaining;

		return true;
	}
	else
	{
		combinedPenetrationModifier = 1.f;

		if (isSolidSurf || isLightSurf)
			finalDamageModifier = 0.99f;
		else
		{
			finalDamageModifier = fminf(enterDamageModifier, exitDamageModifier);
			combinedPenetrationModifier = fminf(enterSurfPenetrationModifier, exitSurfPenetrationModifier);
		}

		if (enterMaterial == exitMaterial && (exitMaterial == CHAR_TEX_METAL || exitMaterial == CHAR_TEX_WOOD))
			combinedPenetrationModifier += combinedPenetrationModifier;

		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr();

		if (sqrt(thickness) <= combinedPenetrationModifier * penetrationPower)
		{
			currentDamage *= finalDamageModifier;
			eyePosition = exitTrace.endpos;
			--possibleHitsRemaining;
			return true;
		}
		return false;
	}
}
std::optional<CAutoWall::wall_pen> CAutoWall::FireBullet(C_BaseCombatWeapon* pWeapon, Vector& direction, float& currentDamage)
{
	if (!pWeapon)
		return std::nullopt;

	auto data = FireBulletData(g_LocalPlayer->GetEyePos());

	data.filter = CTraceFilter();
	data.filter.pSkip = g_LocalPlayer;

	bool sv_penetration_type;

	float currentDistance = 0.f, penetrationPower, penetrationDistance, maxRange, ff_damage_reduction_bullets, ff_damage_bullet_penetration, rayExtension = 40.f;
	Vector eyePosition = g_LocalPlayer->GetEyePos();

	ConVar* penetrationSystem = g_CVar->FindVar(("sv_penetration_type"));
	ConVar* damageReductionBullets = g_CVar->FindVar(("ff_damage_reduction_bullets"));
	ConVar* damageBulletPenetration = g_CVar->FindVar(("ff_damage_bullet_penetration"));

	sv_penetration_type = penetrationSystem->GetBool();
	ff_damage_reduction_bullets = damageReductionBullets->GetFloat();
	ff_damage_bullet_penetration = damageBulletPenetration->GetFloat();

	CCSWeaponInfo* weaponData = pWeapon->GetCSWeaponData();
	CGameTrace enterTrace;
	CTraceFilter filter;

	filter.pSkip = g_LocalPlayer;

	if (!weaponData)
		return std::nullopt;

	maxRange = weaponData->flRange;

	GetBulletTypeParameters(penetrationPower, penetrationDistance, weaponData->szBulletType, sv_penetration_type);

	if (sv_penetration_type)
		penetrationPower = weaponData->flPenetration;

	int possibleHitsRemaining = 4;

	currentDamage = weaponData->iDamage;

	while (possibleHitsRemaining > 0 && currentDamage >= 1.f)
	{
		maxRange -= currentDistance;

		Vector end = eyePosition + direction * maxRange;

		TraceLine(eyePosition, end, MASK_SHOT_HULL | CONTENTS_HITBOX, (IClientEntity*)g_LocalPlayer, &enterTrace);
		ClipTraceToPlayers(eyePosition, end + direction * rayExtension, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace); //  | CONTENTS_HITBOX

		surfacedata_t* enterSurfaceData = g_PhysSurface->GetSurfaceData(enterTrace.surface.surfaceProps);

		float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;

		int enterMaterial = enterSurfaceData->game.material;

		if (enterTrace.fraction == 1.f)
			break;

		currentDistance += enterTrace.fraction * maxRange;

		currentDamage *= pow(weaponData->flRangeModifier, (currentDistance / 500.f));

		if (currentDistance > penetrationDistance && weaponData->flPenetration > 0.f || enterSurfPenetrationModifier < 0.1f)
			break;

		bool canDoDamage = (enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC);
		bool isEnemy = (g_LocalPlayer->m_iTeamNum() != static_cast<C_BasePlayer*>(enterTrace.hit_entity)->m_iTeamNum());

		if (canDoDamage && isEnemy)
		{
			ScaleDamage(enterTrace, weaponData, currentDamage);
			return wall_pen{ currentDamage ,enterTrace.hitbox,enterTrace.hitgroup };
		}

		if (!HandleBulletPenetration(weaponData, enterTrace, eyePosition, direction, possibleHitsRemaining, currentDamage, penetrationPower, sv_penetration_type, ff_damage_reduction_bullets, ff_damage_bullet_penetration))
			break;
	}

	return std::nullopt;
}

float CAutoWall::CanHit(Vector& point)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return -1;

	auto data = FireBulletData(g_LocalPlayer->GetEyePos());
	data.filter = CTraceFilter();
	data.filter.pSkip = g_LocalPlayer;

	QAngle angles;
	Vector direction;
	Vector tmp = point - g_LocalPlayer->GetEyePos();

	float currentDamage = 0;

	Math::VectorAngles(tmp, angles);
	Math::AngleVectors(angles, direction);

	direction.NormalizeInPlace();

	if (FireBullet(g_LocalPlayer->m_hActiveWeapon(), direction, currentDamage).has_value())
	{
		return currentDamage;
	}

	return -1;
}

std::optional<CAutoWall::wall_pen> CAutoWall::CanHitDetail(Vector& point)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return std::nullopt;

	auto data = FireBulletData(g_LocalPlayer->GetEyePos());
	data.filter = CTraceFilter();
	data.filter.pSkip = g_LocalPlayer;

	QAngle angles;
	Vector direction;
	Vector tmp = point - g_LocalPlayer->GetEyePos();

	float currentDamage = 0;

	Math::VectorAngles(tmp, angles);
	Math::AngleVectors(angles, direction);

	direction.NormalizeInPlace();

	return FireBullet(g_LocalPlayer->m_hActiveWeapon(), direction, currentDamage);
}


CAutoWall g_AutoWall;