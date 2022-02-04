#include "../valve_sdk/csgostructs.hpp"
#include "../render.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include <algorithm>

static int type = 0; 

class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;

	Visuals();
	~Visuals();
public:
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
		} ctx;

		bool Begin(C_BasePlayer* pl);
		void RenderBox(C_BaseEntity* entity);
		void RenderName(C_BaseEntity* entity);
		void RenderWeaponName(C_BaseEntity* entity);
		void RenderAmmo(C_BaseEntity* entity);
		void RenderHealth(C_BaseEntity* entity);
		void RenderArmour();
	};
	void RenderWeapon(C_BaseCombatWeapon* ent);
	void RenderPlantedC4(C_BaseEntity* ent);
	void RenderItemEsp(C_BaseEntity* ent);
	void DrawKeyPresses();
	void ThirdPerson();
public:
	void AddToDrawList();
};