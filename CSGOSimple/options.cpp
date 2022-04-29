#include <ShlObj.h>
#include <filesystem>
#include "options.hpp"
#include "valve_sdk/misc/Enums.hpp"
#include "valve_sdk/sdk.hpp"
#include "helpers/logs.hpp"

const std::map<int, const char*> config_names = {
	{WEAPON_CZ75A, "CZ75 Auto"},
	{WEAPON_DEAGLE, "Desert Eagle"},
	{WEAPON_ELITE, "Dual Berettas"},
	{WEAPON_FIVESEVEN, "Five-SeveN"},
	{WEAPON_HKP2000, "P2000"},
	{WEAPON_P250, "P250"},
	{WEAPON_USP_SILENCER, "USP-S"},
	{WEAPON_TEC9, "Tec-9"},
	{WEAPON_REVOLVER, "R8 Revolver"},
	{WEAPON_GLOCK, "Glock-18"},

	{WEAPON_MAG7, "MAG-7"},
	{WEAPON_NOVA, "Nova"},
	{WEAPON_SAWEDOFF, "Sawed-Off"},
	{WEAPON_XM1014, "XM1014"},

	{WEAPON_UMP45, "UMP-45"},
	{WEAPON_P90, "P90"},
	{WEAPON_BIZON, "PP-Bizon"},
	{WEAPON_MAC10, "MAC-10"},
	{WEAPON_MP7, "MP7"},
	{WEAPON_MP9, "MP9"},
	{WEAPON_MP5, "MP5-SD"},

	{WEAPON_M249, "M249"},
	{WEAPON_NEGEV, "Negev"},

	{WEAPON_AK47, "AK-47"},
	{WEAPON_AUG, "AUG"},
	{WEAPON_GALILAR, "Galil AR"},
	{WEAPON_M4A1_SILENCER, "M4A1-S"},
	{WEAPON_M4A1, "M4A4"},
	{WEAPON_SG556, "SG 553"},
	{WEAPON_FAMAS, "FAMAS"},

	{WEAPON_AWP, "AWP"},
	{WEAPON_G3SG1, "G3SG1"},
	{WEAPON_SCAR20, "SCAR-20"},
	{WEAPON_SSG08, "SSG 08"},

	{WEAPON_KNIFE, "Knife"},
	{WEAPON_KNIFE_T, "TKnife"},

	{GLOVE_T_SIDE, "Glove"},
};

const std::map<int, const char*> k_weapon_names = {
	{WEAPON_KNIFE, "knife ct"},
	{WEAPON_KNIFE_T, "knife t"},
	{GLOVE_CT_SIDE, "glove ct"},
	{GLOVE_T_SIDE, "glove t"},
	{61, "usp"},
	{32, "p2000"},
	{4, "glock-18"},
	{2, "dual berettas"},
	{36, "p250"},
	{3, "five-seven"},
	{30, "tec-9"},
	{63, "cz75 auto"},
	{64, "r8 revolver"},
	{1, "deagle"},

	{34, "mp9"},
	{17, "mac-10"},
	{23, "mp5-sd"},
	{33, "mp7"},
	{24, "ump-45"},
	{19, "p90"},
	{26, "pp-bizon"},

	{7, "ak-47"},
	{60, "m4a1-s"},
	{16, "m4a4"},
	{8, "aug"},
	{39, "sg553"},
	{10, "famas"},
	{13, "galil"},

	{40, "ssg08"},
	{38, "scar-20"},
	{9, "awp"},
	{11, "g3sg1"},

	{14, "m249"},
	{28, "negev"},

	{27, "mag-7"},
	{35, "nova"},
	{29, "sawed-off"},
	{25, "xm1014"},
};

void Options::SetupValue(int& value, std::string category, std::string name)
{
	ints.push_back(new ConfigValue<int>(std::move(category), std::move(name), &value));
}

void Options::SetupValue(bool& value, std::string category, std::string name)
{
	bools.push_back(new ConfigValue<bool>(std::move(category), std::move(name), &value));
}

void Options::SetupValue(float& value, std::string category, std::string name)
{
	floats.push_back(new ConfigValue<float>(std::move(category), std::move(name), &value));
}

void Options::SetupColor(Color& value, const std::string& name)
{
	SetupValue(value._CColor[0], "Colors", name + "_r");
	SetupValue(value._CColor[1], "Colors", name + "_g");
	SetupValue(value._CColor[2], "Colors", name + "_b");
	SetupValue(value._CColor[3], "Colors", name + "_a");
}

int Options::GetIntValue(std::string category, std::string name) {
	bool is_have = false;
	for (auto value : ints) {
		if (value->category == category && value->name == name) {
			is_have = true;
			return *value->value;
		}
	}
	if (!is_have)
		Logs::Get().Create("Unknow element: " + category + ":" + name);
}

bool Options::GetBoolValue(std::string category, std::string name) {
	bool is_have = false;
	for (auto value : bools) {
		if (value->category == category && value->name == name) {
			is_have = true;
			return *value->value;
		}
	}
	if (!is_have)
		Logs::Get().Create("Unknow element: " + category + ":" + name);
}

float Options::GetFloatValue(std::string category, std::string name) {
	bool is_have = false;
	for (auto value : floats) {
		if (value->category == category && value->name == name) {
			is_have = true;
			return *value->value;
		}
	}
	if (!is_have)
		Logs::Get().Create("Unknow element: " + category + ":" + name);
}

void Options::AddCheckbox(std::string name) {
	bool_elements[name] = false;
	bools.push_back(new ConfigValue<bool>("profile.lua_elements", std::move(name), &bool_elements[name], 0, 0));
}

void Options::AddSliderInt(std::string name, int min, int max) {
	int_elements[name] = min;
	ints.push_back(new ConfigValue<int>("profile.lua_elements", std::move(name), &int_elements[name], min, max));
}

void Options::AddSliderFloat(std::string name, float min, float max) {
	float_elements[name] = min;
	floats.push_back(new ConfigValue<float>("profile.lua_elements", std::move(name), &float_elements[name], min));
}


void Options::SetupSettings()
{
	SetupValue(g_Options.legit_enabled, "legitbot", "enabled");
	for (int i = 0; i < 5; i++)
	{
		auto aimbot = &g_Options.legitbot[i];

		SetupValue(aimbot->enabled, "legitbot[" + std::to_string(i) + "]", "enabled");
		SetupValue(aimbot->deathmatch, "legitbot[" + std::to_string(i) + "]", "deathmatch");
		SetupValue(aimbot->silent, "legitbot[" + std::to_string(i) + "]", "silent");

		SetupValue(aimbot->flash_check, "legitbot[" + std::to_string(i) + "]", "flash_check");
		SetupValue(aimbot->smoke_check, "legitbot[" + std::to_string(i) + "]", "smoke_check");
		SetupValue(aimbot->autopistol, "legitbot[" + std::to_string(i) + "]", "auto_pistol");

		SetupValue(aimbot->fov, "legitbot[" + std::to_string(i) + "]", "fov");
		SetupValue(aimbot->silent_fov, "legitbot[" + std::to_string(i) + "]", "silent_fov");
		SetupValue(aimbot->smooth, "legitbot[" + std::to_string(i) + "]", "smooth");

		SetupValue(aimbot->kill_delay, "legitbot[" + std::to_string(i) + "]", "kill_delay");
		SetupValue(aimbot->shot_delay, "legitbot[" + std::to_string(i) + "]", "Shot Delay");

		SetupValue(aimbot->hitboxes.head, "legitbot[" + std::to_string(i) + "].hitbox", "head");
		SetupValue(aimbot->hitboxes.chest, "legitbot[" + std::to_string(i) + "].hitbox", "chest");
		SetupValue(aimbot->hitboxes.hands, "legitbot[" + std::to_string(i) + "].hitbox", "hands");
		SetupValue(aimbot->hitboxes.legs, "legitbot[" + std::to_string(i) + "].hitbox", "legs");

		SetupValue(aimbot->rcs.enabled, "legitbot[" + std::to_string(i) + "].rcs", "enabled");
		SetupValue(aimbot->rcs.start, "legitbot[" + std::to_string(i) + "].rcs", "start");
		SetupValue(aimbot->rcs.x, "legitbot[" + std::to_string(i) + "].rcs", "x");
		SetupValue(aimbot->rcs.y, "legitbot[" + std::to_string(i) + "].rcs", "y");

		SetupValue(aimbot->autowall.enabled, "legitbot.autowall" + i, "enabled");
		SetupValue(aimbot->autowall.min_damage, "legitbot.autowall" + i, "damage");

	}

	SetupValue(g_Options.autofire_enabled, "legitbot.autofire", "enabled");
	SetupValue(g_Options.autofire_key, "legitbot.autofire", "key"); SetupValue(g_Options.autofire_type, "legitbot.autofire", "type");


	SetupValue(g_Options.rage_enabled, "ragebot", "enabled");
	SetupValue(g_Options.roll_resolver_key, "ragebot", "Roll Resolver Key"); SetupValue(g_Options.roll_resolver_type, "RAGE", "Roll Resolver Type");
	for (int i = 0; i < 6; i++)
	{
		auto aimbot = &g_Options.ragebot[i];

		SetupValue(aimbot->enabled, "ragebot[" + std::to_string(i) + "]", "enabled");
		SetupValue(aimbot->autoshot, "ragebot[" + std::to_string(i) + "]", "autoshot");
		SetupValue(aimbot->autowall, "ragebot[" + std::to_string(i) + "]", "autowall");
		SetupValue(aimbot->autostop, "ragebot[" + std::to_string(i) + "]", "autostop");
		SetupValue(aimbot->silent, "ragebot[" + std::to_string(i) + "]", "silent");

		SetupValue(aimbot->damage, "ragebot[" + std::to_string(i) + "]", "damage");
		SetupValue(aimbot->hitchance, "ragebot[" + std::to_string(i) + "]", "hitchance");
		
		SetupValue(aimbot->multipoint_body, "ragebot[" + std::to_string(i) + "].multipoint", "body");
		SetupValue(aimbot->multipoint_head, "ragebot[" + std::to_string(i) + "].multipoint", "head");

		SetupValue(aimbot->hitboxes.head, "ragebot[" + std::to_string(i) + "].hitbox", "head");
		SetupValue(aimbot->hitboxes.upper_chest, "ragebot[" + std::to_string(i) + "].hitbox", "upper_chest");
		SetupValue(aimbot->hitboxes.chest, "ragebot[" + std::to_string(i) + "].hitbox", "chest");
		SetupValue(aimbot->hitboxes.lower_chest, "ragebot[" + std::to_string(i) + "].hitbox", "lower_chest");
		SetupValue(aimbot->hitboxes.hands, "ragebot[" + std::to_string(i) + "].hitbox", "hands");
		SetupValue(aimbot->hitboxes.legs, "ragebot[" + std::to_string(i) + "].hitbox", "legs");

	}

	SetupValue(g_Options.antiaim, "antiaim", "Enabled");
	SetupValue(g_Options.antiaim_flip_key, "antiaim", "flip_key"); SetupValue(g_Options.antiaim_flip_type, "antiaim", "flip_type");

	SetupValue(g_Options.antiaim_pitch, "antiaim", "pitch");
	SetupValue(g_Options.antiaim_yaw, "antiaim", "yaw");
	SetupValue(g_Options.antiaim_dsy, "antiaim", "desync");

	SetupValue(g_Options.fakelag, "antiaim.fakelag", "enabled");
	SetupValue(g_Options.fakelag_amount, "antiaim.fakelag", "amount");

	for (auto& [key, val] : k_weapon_names) {
		auto& option = g_Options.changers.skin.m_items[key];
		SetupValue(option.definition_vector_index, val, "d_vec_index");
		SetupValue(option.definition_index, val, "d_index");
		SetupValue(option.paint_kit_vector_index, val, "pk_vec_index");
		SetupValue(option.paint_kit_index, val, "pk_index");
		SetupValue(option.definition_override_index, val, "do_index");
		SetupValue(option.definition_override_vector_index, val, "do_vec_index");
		//SetupValue(option.seed, val, "seed");
		//SetupValue(option.enable_stat_track, val, "stattrack");
		//SetupValue(option.stat_trak, val, "stat_trak_val");
		//SetupValue(option.wear, val, "wear");
	}



	SetupValue(g_Options.esp_player_boxes, "visuals.enemy.esp", "boxes");
	SetupValue(g_Options.esp_player_boxesOccluded, "visuals.enemy.esp", "occluded");
	SetupValue(g_Options.esp_player_names, "visuals.enemy.esp", "name");
	SetupValue(g_Options.esp_player_health, "visuals.enemy.esp", "health");
	SetupValue(g_Options.esp_player_weapons, "visuals.enemy.esp", "weapon");
	SetupValue(g_Options.esp_dropped_weapons, "visuals.esp", "dropped_weapons");
	SetupValue(g_Options.chams_player_enabled, "visuals.enemy", "enabled_chams");
	SetupValue(g_Options.chams_player_ignorez, "visuals.enemy", "enabled_chams_occluded");
	SetupValue(g_Options.glow_enabled, "visuals.enemy", "glow_enabled");
	SetupValue(g_Options.glow_enemiesOC, "visuals.enemy", "glow_enabled_occluded");
	SetupValue(g_Options.glow_enemies_type, "visuals.enemy", "glow_type");
	SetupValue(g_Options.chams_player_visible, "visuals.enemy", "Type visible");
	SetupValue(g_Options.chams_player_occluded, "visuals.enemy", "Type occluded");
	SetupValue(g_Options.player_enemies_shine, "visuals.enemy", "visible_shine");
	SetupValue(g_Options.chams_desync_enabled, "visuals.local.desync", "enabled");
	SetupValue(g_Options.chams_desync, "visuals.local.desync", "type");
	SetupColor(g_Options.player_enemy_visible_shine, "enemy_visible_shine");
	SetupColor(g_Options.color_esp_enemy_visible, "enemy_visible");
	SetupColor(g_Options.color_esp_enemy_occluded, "enemy_occluded");
	SetupColor(g_Options.color_chams_player_enemy_visible, "enemy_visible_chams");
	SetupColor(g_Options.color_chams_player_enemy_occluded, "enemy_occluded_chams");
	SetupColor(g_Options.color_chams_player_desync, "local_desync");
	SetupColor(g_Options.color_glow_enemy, "enemy_visible_glow");
	SetupColor(g_Options.color_glow_enemyOC, "enemy_occluded_glow");
	SetupValue(g_Options.draw_multipoints, "visuals", "draw_multipoints");

	SetupValue(g_Options.enable_nightmode, "visuals.nightmode", "enabled");
	SetupColor(g_Options.nightmode_color, "nightmode_color");

	SetupValue(g_Options.enable_fog, "visuals.fog", "enabled");
	SetupValue(g_Options.fog_density, "visuals.fog", "densivity");
	SetupValue(g_Options.fog_start_distance, "visuals.fog", "start");
	SetupValue(g_Options.fog_end_distance, "visuals.fog", "end");
	SetupColor(g_Options.fog_color, "fog_color");

	SetupValue(g_Options.remove_smoke, "visuals.removals", "smoke");
	SetupValue(g_Options.remove_scope, "visuals.removals", "scope");
	SetupValue(g_Options.remove_visualrecoil, "visuals.removals", "visual_recoil");
	SetupValue(g_Options.remove_post_processing, "visuals.removals", "post_processing");
	SetupValue(g_Options.remove_zoom, "visual.removals", "zoom");
	SetupValue(g_Options.remove_scope, "visuals.removals", "scope");
	SetupValue(g_Options.remove_flash, "visuals.removals", "flash");

	SetupValue(g_Options.aspect_ratio, "aspectratio", "enabled");
	SetupValue(g_Options.aspect_ratio_scale, "aspectratio", "value");



	SetupValue(g_Options.logs, "misc.logs", "enabled");
	SetupValue(g_Options.logs_drawing, "misc.logs", "drawing");

	SetupValue(g_Options.autowalk, "misc", "walk_bot");
	SetupValue(g_Options.misc_showranks, "misc", "rank_reveal");
	SetupValue(g_Options.misc_watermark, "misc", "watermark");
	SetupValue(g_Options.autoaccept, "misc", "auto_accept");
	SetupValue(g_Options.misc_autostrafe, "misc", "auto_strafe");
	SetupValue(g_Options.misc_boostspeed, "misc", "speed_boost");
	SetupValue(g_Options.misc_wasdstrafes, "misc", "wasd_strafer");
	SetupValue(g_Options.misc_bhop, "misc", "bunny_hop");
	SetupValue(g_Options.sniper_xhair, "misc", "sniper_crosshair");
	SetupValue(g_Options.playerModelCT, "misc", "playerModelCT");
	SetupValue(g_Options.playerModelT, "misc", "playerModelT");
	SetupValue(g_Options.misc_thirdperson, "misc.thirdperson", "enabled");
	SetupValue(g_Options.misc_thirdperson_key, "misc.thirdperson", "key");
	SetupValue(g_Options.misc_thirdperson_type, "misc.thirdperson", "type");

}

void Options::Initialize()
{
	CHAR my_documents[MAX_PATH];
	SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, my_documents);
	folder = my_documents + std::string("\\nessless\\");
	CreateDirectoryA(folder.c_str(), nullptr);
	SetupSettings();
}

void Options::SaveSettings(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	for (auto value : ints)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : bools)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());

	for (auto value : floats)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
}

void Options::LoadSettings(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	if (!std::filesystem::exists(file))
		return;

	char value_l[32] = { '\0' };

	for (auto value : ints)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = atoi(value_l);
	}

	for (auto value : bools)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = !strcmp(value_l, "true");
	}

	for (auto value : floats)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = float(atof(value_l));
	}
	g_ClientState->ForceFullUpdate();
}

void Options::DeleteSettings(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	if (!std::filesystem::exists(file))
		return;

	remove(file.c_str());
}