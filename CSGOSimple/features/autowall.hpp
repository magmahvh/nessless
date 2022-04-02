#pragma once
#include "../valve_sdk/sdk.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include <optional>

#define DAMAGE_NO		         0
#define DAMAGE_EVENTS_ONLY	     1	
#define DAMAGE_YES		         2
#define DAMAGE_AIM		         3
#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define	CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E' ///< the egg sacs in the tunnels in ep2.
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z' ///< weird-looking jello effect for advisor shield.

struct FireBulletData
{
	FireBulletData(const Vector& eye_pos) : src(eye_pos) {}

	Vector          src;
	trace_t         enter_trace;
	Vector          direction;
	CTraceFilter    filter;
	float           trace_length;
	float           trace_length_remaining;
	float           current_damage;
	int             penetrate_count;
};

class CAutoWall : public Singleton<CAutoWall>
{
public:
	struct wall_pen
	{
		float damage;
		int hitbox;
		int32_t hitgroup;
	};
	float CanHit(Vector& point);
	std::optional<CAutoWall::wall_pen> CanHitDetail(Vector& point);
private:
	void TraceLine(Vector& absStart, Vector& absEnd, unsigned int mask, IClientEntity* ignore, CGameTrace* ptr);
	void ClipTraceToPlayers(const Vector& absStart, const Vector absEnd, unsigned int mask, ITraceFilter* filter, CGameTrace* tr);
	void GetBulletTypeParameters(float& maxRange, float& maxDistance, char* bulletType, bool sv_penetration_type);
	bool BreakableEntity(IClientEntity* entity);
	void ScaleDamage(CGameTrace& enterTrace, CCSWeaponInfo* weaponData, float& currentDamage);
	bool TraceToExit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction);
	bool HandleBulletPenetration(CCSWeaponInfo* weaponData, CGameTrace& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration);
	std::optional<CAutoWall::wall_pen> FireBullet(C_BaseCombatWeapon* pWeapon, Vector& direction, float& currentDamage);

};

extern CAutoWall g_AutoWall;