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
	for (auto value : ints) {
		if (value->category == category && value->name == name) {
			return *value->value;
		}
	}
	Logs::Get().Create("Unknow element: " + category + ":" + name);
}

int Options::GetBoolValue(std::string category, std::string name) {
	for (auto value : bools) {
		if (value->category == category && value->name == name) {
			return *value->value;
		}
	}
	Logs::Get().Create("Unknow element: " + category + ":" + name);
}

int Options::GetFloatValue(std::string category, std::string name) {
	for (auto value : floats) {
		if (value->category == category && value->name == name) {
			return *value->value;
		}
	}
	Logs::Get().Create("Unknow element: " + category + ":" + name);
}


void Options::SetupWeapons()
{
	SetupValue(g_Options.legit_enabled, "Legitbot", "Enabled Legit");
	SetupValue(g_Options.rage_enabled, "Ragebot", "Enabled Rage");
	SetupValue(g_Options.roll_resolver_key, "Ragebot", "Roll Resolver Key"); SetupValue(g_Options.roll_resolver_type, "RAGE", "Roll Resolver Type");
	for (int i = 0; i < 5; i++)
	{
		auto aimbot = &g_Options.legitbot[i];

		SetupValue(aimbot->enabled, "Legitbot" + i, "Enabled");
		SetupValue(aimbot->deathmatch, "Legitbot" + i, "Deathmatch");
		SetupValue(aimbot->silent, "Legitbot" + i, "Silent");

		SetupValue(aimbot->flash_check, "Legitbot" + i, "Flash Check");
		SetupValue(aimbot->smoke_check, "Legitbot" + i, "Smoke Check");
		SetupValue(aimbot->autopistol, "Legitbot" + i, "AutoPistol");

		SetupValue(aimbot->fov, "Legitbot" + i, "Fov");
		SetupValue(aimbot->silent_fov, "Legitbot" + i, "pSilent Fov");
		SetupValue(aimbot->smooth, "Legitbot" + i, "Smooth");

		SetupValue(aimbot->kill_delay, "Legitbot" + i, "Kill Delay");
		SetupValue(aimbot->shot_delay, "Legitbot" + i, "Shot Delay");

		SetupValue(aimbot->hitboxes.head, "Legitbot" + i, "Head Hitbox");
		SetupValue(aimbot->hitboxes.chest, "Legitbot" + i, "Chest Hitbox");
		SetupValue(aimbot->hitboxes.hands, "Legitbot" + i, "Hands Hitbox");
		SetupValue(aimbot->hitboxes.legs, "Legitbot" + i, "Legs Hitbox");

		SetupValue(aimbot->rcs.enabled, "Legitbot" + i, "RCS Enabled");
		SetupValue(aimbot->rcs.start, "Legitbot" + i, "RCS Start");
		SetupValue(aimbot->rcs.x, "Legitbot" + i, "RCS X");
		SetupValue(aimbot->rcs.y, "Legitbot" + i, "RCS Y");

		SetupValue(aimbot->autowall.enabled, "Legitbot" + i, "AutoWall Enabled");
		SetupValue(aimbot->autowall.min_damage, "Legitbot" + i, "AutoWall MinDamage");

	}

	SetupValue(g_Options.autofire_enabled, "Legitbot", "AutoFire Enabled");
	SetupValue(g_Options.autofire_key, "Legitbot", "Auto Fire Key"); SetupValue(g_Options.autofire_type, "Legitbot", "Auto Fire Type");

	for (int i = 0; i < 6; i++)
	{
		auto aimbot = &g_Options.ragebot[i];

		SetupValue(aimbot->enabled, "Ragebot" + i, "Enabled");
		SetupValue(aimbot->autoshot, "Ragebot" + i, "Autoshot");
		SetupValue(aimbot->autowall, "Ragebot" + i, "Autowall");
		SetupValue(aimbot->autostop, "Ragebot" + i, "Autostop");
		SetupValue(aimbot->silent, "Ragebot" + i, "Silent");

		SetupValue(aimbot->damage, "Ragebot" + i, "Damage");
		SetupValue(aimbot->hitchance, "Ragebot" + i, "Hitchance");
		
		SetupValue(aimbot->multipoint_body, "Ragebot" + i, "Body multipoint");
		SetupValue(aimbot->multipoint_head, "Ragebot" + i, "Head multipoint");

		SetupValue(aimbot->hitboxes.head, "Ragebot" + i, "Head Hitbox");
		SetupValue(aimbot->hitboxes.upper_chest, "Ragebot" + i, "Upper Chest Hitbox");
		SetupValue(aimbot->hitboxes.chest, "Ragebot" + i, "Chest Hitbox");
		SetupValue(aimbot->hitboxes.lower_chest, "Ragebot" + i, "Lower Chest Hitbox");
		SetupValue(aimbot->hitboxes.hands, "Ragebot" + i, "Hands Hitbox");
		SetupValue(aimbot->hitboxes.legs, "Ragebot" + i, "Legs Hitbox");

	}

	SetupValue(g_Options.antiaim, "AntiAim", "Enabled");
	SetupValue(g_Options.antiaim_flip_key, "RAGE", "AA Flip Key"); SetupValue(g_Options.antiaim_flip_type, "RAGE", "AA Flip Type");

	SetupValue(g_Options.antiaim_pitch, "AntiAim", "Pitch");
	SetupValue(g_Options.antiaim_yaw, "AntiAim", "Yaw");
	SetupValue(g_Options.antiaim_dsy, "AntiAim", "Dsy");

	SetupValue(g_Options.fakelag, "AntiAim", "Fakelag");
	SetupValue(g_Options.fakelag_amount, "AntiAim", "Fakelag amount");

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

}

void Options::SetupVisuals()
{
	SetupValue(g_Options.esp_player_boxes, "Visuals", "Boxes");
	SetupValue(g_Options.esp_player_boxesOccluded, "Visuals", "Occluded");
	SetupValue(g_Options.esp_player_names, "Visuals", "Names");
	SetupValue(g_Options.esp_player_health, "Visuals", "Health");
	SetupValue(g_Options.esp_player_weapons, "Visuals", "Weapon");
	SetupValue(g_Options.esp_dropped_weapons, "Visuals", "Dropped Weapons");
	SetupValue(g_Options.chams_player_enabled, "Visuals", "Enabled ch");
	SetupValue(g_Options.chams_player_ignorez, "Visuals", "Occluded ch");
	SetupValue(g_Options.glow_enabled, "Visuals", "Enabled gl");
	SetupValue(g_Options.glow_enemiesOC, "Visuals", "Occluded gl");
	SetupValue(g_Options.glow_enemies_type, "Visuals", "glow_enemies_type");
	SetupValue(g_Options.chams_player_flat, "Visuals", "Flat");
	SetupValue(g_Options.player_enemies_shine, "Visuals", "Visible shine");
	SetupColor(g_Options.player_enemy_visible_shine, "color Visible");
	SetupColor(g_Options.player_enemy_flat, "color Visible");
	SetupColor(g_Options.color_esp_enemy_visible, "Enemies Visible");
	SetupColor(g_Options.color_esp_enemy_occluded, "Enemies Occluded");
	SetupColor(g_Options.color_chams_player_enemy_visible, "Enemy Visible ch");
	SetupColor(g_Options.color_chams_player_enemy_occluded, "Enemy Occluded ch");
	SetupColor(g_Options.color_glow_enemy, "Enemy Visible");
	SetupColor(g_Options.color_glow_enemyOC, "Enemy Occluded");

	SetupValue(g_Options.enable_nightmode, "Visuals", "Nightmode");
	SetupColor(g_Options.nightmode_color, "Nightmode color");

	SetupValue(g_Options.enable_fog, "Visuals", "Fog");
	SetupValue(g_Options.fog_density, "Visuals", "Fog densivity");
	SetupValue(g_Options.fog_start_distance, "Visuals", "Fog start");
	SetupValue(g_Options.fog_end_distance, "Visuals", "Fog end");
	SetupColor(g_Options.fog_color, "Fog color");

	SetupValue(g_Options.remove_smoke, "Visuals", "Remove smoke");
	SetupValue(g_Options.remove_scope, "Visuals", "Remove scope");
	SetupValue(g_Options.remove_visualrecoil, "Visuals", "Remove visual recoil");
	SetupValue(g_Options.remove_post_processing, "Visuals", "Remove post processing");
	SetupValue(g_Options.remove_zoom, "Visuals", "Remove zoom");
	SetupValue(g_Options.remove_scope, "Visuals", "Remove scope");
	SetupValue(g_Options.remove_flash, "Visuals", "Remove flash");
}

void Options::SetupMisc()
{
	SetupValue(g_Options.logs, "Misc", "Logs");
	SetupValue(g_Options.logs_drawing, "Misc", "Logs Drawing");

	SetupValue(g_Options.autowalk, "Misc", "Walk bot");
	SetupColor(g_Options.Velocitycol, "Velocity");
	SetupValue(g_Options.misc_showranks, "Misc", "Rank reveal");
	SetupValue(g_Options.misc_watermark, "Misc", "Watermark");
	SetupValue(g_Options.Velocity, "Misc", "Velocity");
	SetupValue(g_Options.outline, "Misc", "Outline");
	SetupValue(g_Options.lastjump, "Misc", "Last jump");
	SetupValue(g_Options.lastjumpoutline, "Misc", "Last jump outline");
	SetupValue(g_Options.autoaccept, "Misc", "Auto accept");
	SetupValue(g_Options.misc_autostrafe, "Misc", "Auto strafe");
	SetupValue(g_Options.misc_boostspeed, "Misc", "Speed boost");
	SetupValue(g_Options.misc_wasdstrafes, "Misc", "WASD strafer");
	SetupValue(g_Options.misc_bhop, "Misc", "Bunny hop");
	SetupValue(g_Options.edge_bug, "Misc", "Edge bug");
	SetupValue(g_Options.edge_bug_key, "Misc", "edge_bug_key");
	SetupValue(g_Options.edgejump.enabled, "Misc", "Edge jump");
	SetupValue(g_Options.edgejump.hotkey, "Misc", "edgejump_key");
	SetupValue(g_Options.edgejump.edge_jump_duck_in_air, "Misc", "Duck in Air");
	SetupValue(g_Options.sniper_xhair, "Misc", "Sniper crosshair");
	SetupValue(g_Options.playerModelCT, "Misc", "playerModelCT");
	SetupValue(g_Options.playerModelT, "Misc", "playerModelT");
	SetupValue(g_Options.misc_thirdperson_bind, "Misc", "Thirdperson bind");
	SetupValue(g_Options.misc_thirdperson_dist, "Misc", "Thirdperson distance");

}

void Options::Initialize()
{
	CHAR my_documents[MAX_PATH];
	SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, my_documents);
	folder = my_documents + std::string("\\nessless\\");
	CreateDirectoryA(folder.c_str(), nullptr);
	SetupWeapons();
	SetupVisuals();
	SetupMisc();
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