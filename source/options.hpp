#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "valve_sdk/Misc/Color.hpp"

#define A( s ) #s
#define OPTION(type, var, val) Var<type> var = {A(var), val}

template <typename T = bool>
class Var {
public:
	std::string name;
	std::shared_ptr<T> value;
	int32_t size;
	Var(std::string name, T v) : name(name) {
		value = std::make_shared<T>(v);
		size = sizeof(T);
	}
	operator T() { return *value; }
	operator T*() { return &*value; }
	operator T() const { return *value; }
	//operator T*() const { return value; }
};

class Options
{
public:
		//
		// RAGE
		// 
		OPTION(bool, enabled_ragebot, false);
		OPTION(bool, autoscope, false);

		OPTION(bool, doubletap, false);
		OPTION(int, doubletap_bind, 0);

		struct Weapon
		{
			/*hitboxes*/
			OPTION(bool, head, false);
			OPTION(bool, chest, false);
			OPTION(bool, body, false);
			OPTION(bool, arms, false);
			OPTION(bool, legs, false);
			OPTION(bool, foot, false);
			/*hitboxes*/

			OPTION(int, hitchance, 1);
			OPTION(int, multipoints, 1);
			OPTION(int, damage, 1);
			OPTION(int, damage_override_bind, 0);
			OPTION(int, damage_override, 1);
			OPTION(bool, rcs, false);
			OPTION(bool, autostop, false);

		} Weapon[7];

		//
		// AA
		//
		OPTION(bool, staticLegs, false);
		OPTION(bool, slidewalk, false);

		// 
		// ESP
		// 

		OPTION(bool, enabled_esp, false);
		OPTION(bool, esp_player_boxes, false); OPTION(bool, esp_player_boxes_outline, false);
		OPTION(bool, esp_player_weapons, false);
		OPTION(bool, esp_player_names, false);
		OPTION(bool, esp_player_health, false); OPTION(bool, esp_player_hp_line, false); OPTION(Color, esp_player_hp_health, Color(80, 185, 20));
		OPTION(bool, esp_player_armour, false); OPTION(Color, esp_player_armour_color, Color(75, 125, 235));
		OPTION(bool, esp_player_ammo, false); OPTION(Color, esp_player_ammo_color, Color(15, 55, 245));

		/* chams */
		OPTION(bool, chams_player_enabled, false);
		OPTION(bool, chams_player_ignorez, false);
		OPTION(int, chams_player_type, 0);
		
		OPTION(bool, chams_arms_enabled, false);
		OPTION(int, chams_arm_type, 0);

		OPTION(Color, color_chams_player_enemy_visible, Color(255, 0, 0));
		OPTION(Color, color_chams_player_enemy_occluded, Color(255, 128, 0));
		OPTION(Color, color_chams_arms, Color(0, 128, 255));
		/* chams */

		OPTION(bool, grenades, false);
		OPTION(Color, grenades_color, Color(120, 140, 230));

		/* indicators */
		OPTION(bool, watermark, false);
		OPTION(Color, color_watermark, Color(0, 128, 255)); // no menu config cuz its useless
		/* indicators */

		OPTION(bool, thirdperson, false);
		OPTION(int, thirdperson_dist, 100);
		OPTION(int, thirdperson_bind, 0);

		OPTION(bool, esp_dropped_weapons, false);
		OPTION(bool, esp_planted_c4, false);
		OPTION(bool, esp_items, false);
		
		/*scope remove*/
		OPTION(bool, sniper_crosshair, false);
		OPTION(bool, scope_remove, false);
		/*scope remove*/

		//
		// MISC
		//

		OPTION(float, misc_aspect_ratio, 0);
		OPTION(bool, bhop, false);


		//
		// CFG
		//

		OPTION(Color, background_menu_clr, Color(32, 32, 32));
		OPTION(Color, child_bg_menu_clr, Color(39, 39, 39));
		OPTION(Color, header_bg_menu_clr, Color(39, 39, 39));
		OPTION(Color, line_header_menu_clr, Color(46, 46, 46));
		OPTION(Color, text_menu_clr, Color(255, 255, 255));
};

inline Options g_Options;
inline bool   g_Unload;

/*
csgo->local->m_flPoseParameter()[0] = 0;

	static bool jitter = false;

	if (jitter) {
		if (csgo->cmd->forwardmove > 0.f){csgo->cmd->buttons |= IN_BACK; csgo->cmd->buttons &= ~IN_FORWARD;}
		if (csgo->cmd->forwardmove < 0.f){csgo->cmd->buttons |= IN_FORWARD; csgo->cmd->buttons &= ~IN_BACK;}
		if (csgo->cmd->sidemove < 0.f){csgo->cmd->buttons |= IN_MOVERIGHT; csgo->cmd->buttons &= ~IN_MOVELEFT;}
		if (csgo->cmd->sidemove > 0.f){csgo->cmd->buttons |= IN_MOVELEFT; csgo->cmd->buttons &= ~IN_MOVERIGHT;}
	}
	else {
		csgo->cmd->buttons &= ~IN_MOVERIGHT;
		csgo->cmd->buttons &= ~IN_MOVELEFT;
		csgo->cmd->buttons &= ~IN_FORWARD;
		csgo->cmd->buttons &= ~IN_BACK;
		if (csgo->cmd->forwardmove > 0.f) csgo->cmd->buttons |= IN_FORWARD;
		else if (csgo->cmd->forwardmove < 0.f) csgo->cmd->buttons |= IN_BACK;
		if (csgo->cmd->sidemove > 0.f) csgo->cmd->buttons |= IN_MOVERIGHT;
		else if (csgo->cmd->sidemove < 0.f) csgo->cmd->buttons |= IN_MOVELEFT;
	}
	if(csgo->client_state->iChokedCommands == 0) jitter = !jitter;
csgo->local->m_flPoseParameter()[0] = 0;
	if (csgo->client_state->iChokedCommands == 0) {
		if (csgo->cmd->forwardmove > 0.f){csgo->cmd->buttons |= IN_BACK; csgo->cmd->buttons &= ~IN_FORWARD;}
		if (csgo->cmd->forwardmove < 0.f){csgo->cmd->buttons |= IN_FORWARD; csgo->cmd->buttons &= ~IN_BACK;}
		if (csgo->cmd->sidemove < 0.f){csgo->cmd->buttons |= IN_MOVERIGHT; csgo->cmd->buttons &= ~IN_MOVELEFT;}
		if (csgo->cmd->sidemove > 0.f){csgo->cmd->buttons |= IN_MOVELEFT; csgo->cmd->buttons &= ~IN_MOVERIGHT;}
	}
	else {
		csgo->cmd->buttons &= ~IN_MOVERIGHT;
		csgo->cmd->buttons &= ~IN_MOVELEFT;
		csgo->cmd->buttons &= ~IN_FORWARD;
		csgo->cmd->buttons &= ~IN_BACK;
		if (csgo->cmd->forwardmove > 0.f) csgo->cmd->buttons |= IN_FORWARD;
		else if (csgo->cmd->forwardmove < 0.f) csgo->cmd->buttons |= IN_BACK;
		if (csgo->cmd->sidemove > 0.f) csgo->cmd->buttons |= IN_MOVERIGHT;
		else if (csgo->cmd->sidemove < 0.f) csgo->cmd->buttons |= IN_MOVELEFT;
	}
*/