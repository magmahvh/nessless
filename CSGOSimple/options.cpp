#include <ShlObj.h>
#include <filesystem>
#include "options.hpp"
#include "valve_sdk/misc/Enums.hpp"
#include "valve_sdk/sdk.hpp"

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
/*void Options::SetupValue(char value, std::string category, std::string name)
{
	chars.push_back(new ConfigValue<char>(std::move(category), std::move(name), &value));
}*/

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



void Options::SetupWeapons()
{
	SetupValue(g_Options.legit_enabled, "LEGITON", "Enabled Legit");
	SetupValue(g_Options.rage_enabled, "RAGEON", "Enabled Rage");
	for (int i = 0; i < 5; i++)
	{
		auto aimbot = &g_Options.legitbot[i];

		SetupValue(aimbot->enabled, "LEGITBOTWPN" + i, "Enabled");
		SetupValue(aimbot->deathmatch, "LEGITBOTWPN" + i, "Deathmatch");
		//SetupValue(aimbot->silent, key, "pSilent");
		SetupValue(aimbot->silent, "LEGITBOTWPN" + i, "Silent");

		SetupValue(aimbot->flash_check, "LEGITBOTWPN" + i, "Flash Check");
		SetupValue(aimbot->smoke_check, "LEGITBOTWPN" + i, "Smoke Check");
		SetupValue(aimbot->autopistol, "LEGITBOTWPN" + i, "AutoPistol");

		SetupValue(aimbot->fov, "LEGITBOTWPN" + i, "Fov");
		SetupValue(aimbot->silent_fov, "LEGITBOTWPN" + i, "pSilent Fov");
		SetupValue(aimbot->smooth, "LEGITBOTWPN" + i, "Smooth");

		SetupValue(aimbot->kill_delay, "LEGITBOTWPN" + i, "Kill Delay");
		SetupValue(aimbot->shot_delay, "LEGITBOTWPN" + i, "Shot Delay");

		SetupValue(aimbot->hitboxes.head, "LEGITBOTWPN" + i, "Head Hitbox");
		SetupValue(aimbot->hitboxes.chest, "LEGITBOTWPN" + i, "Chest Hitbox");
		SetupValue(aimbot->hitboxes.hands, "LEGITBOTWPN" + i, "Hands Hitbox");
		SetupValue(aimbot->hitboxes.legs, "LEGITBOTWPN" + i, "Legs Hitbox");

		SetupValue(aimbot->rcs.enabled, "LEGITBOTWPN" + i, "RCS Enabled");
		SetupValue(aimbot->rcs.start, "LEGITBOTWPN" + i, "RCS Start");
		SetupValue(aimbot->rcs.x, "LEGITBOTWPN" + i, "RCS X");
		SetupValue(aimbot->rcs.y, "LEGITBOTWPN" + i, "RCS Y");

		SetupValue(aimbot->autowall.enabled, "LEGITBOTWPN" + i, "AutoWall Enabled");
		SetupValue(aimbot->autowall.min_damage, "LEGITBOTWPN" + i, "AutoWall MinDamage");

		SetupValue(aimbot->autofire.enabled, "LEGITBOTWPN" + i, "AutoFire Enabled");
		SetupValue(aimbot->autofire.hotkey, "LEGITBOTWPN" + i, "AutoFire Hotkey");

	}

	for (int i = 0; i < 6; i++)
	{
		auto aimbot = &g_Options.ragebot[i];

		SetupValue(aimbot->enabled, "RAGEBOTWPN" + i, "Enabled");
		SetupValue(aimbot->autoshot, "RAGEBOTWPN" + i, "Autoshot");
		SetupValue(aimbot->autowall, "RAGEBOTWPN" + i, "Autowall");
		SetupValue(aimbot->autostop, "RAGEBOTWPN" + i, "Autostop");
		SetupValue(aimbot->silent, "RAGEBOTWPN" + i, "Silent");

		SetupValue(aimbot->damage, "RAGEBOTWPN" + i, "Damage");
		SetupValue(aimbot->hitchance, "RAGEBOTWPN" + i, "Hitchance");

		SetupValue(aimbot->hitboxes.head, "RAGEBOTWPN" + i, "Head Hitbox");
		SetupValue(aimbot->hitboxes.upper_chest, "RAGEBOTWPN" + i, "Upper Chest Hitbox");
		SetupValue(aimbot->hitboxes.chest, "RAGEBOTWPN" + i, "Chest Hitbox");
		SetupValue(aimbot->hitboxes.lower_chest, "RAGEBOTWPN" + i, "Lower Chest Hitbox");
		SetupValue(aimbot->hitboxes.hands, "RAGEBOTWPN" + i, "Hands Hitbox");
		SetupValue(aimbot->hitboxes.legs, "RAGEBOTWPN" + i, "Legs Hitbox");

	}

	SetupValue(g_Options.antiaim, "AntiAim", "Enabled");
	SetupValue(g_Options.antiaim_flip, "AntiAim", "Flip");

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
	SetupValue(g_Options.esp_player_boxes, "ESP", "Boxes");
	SetupValue(g_Options.esp_player_boxesOccluded, "ESP", "Occluded");
	SetupValue(g_Options.esp_player_names, "ESP", "Names");
	SetupValue(g_Options.esp_player_health, "ESP", "Health");
	SetupValue(g_Options.esp_player_weapons, "ESP", "Weapon");
	SetupValue(g_Options.esp_dropped_weapons, "ESP", "Dropped Weapons");
	SetupValue(g_Options.chams_player_enabled, "ESP", "Enabled ch");
	SetupValue(g_Options.chams_player_ignorez, "ESP", "Occluded ch");
	SetupValue(g_Options.glow_enabled, "ESP", "Enabled gl");
	SetupValue(g_Options.glow_enemiesOC, "ESP", "Occluded gl");
	SetupValue(g_Options.glow_enemies_type, "ESP", "glow_enemies_type");
	SetupValue(g_Options.chams_player_flat, "ESP", "Flat");
	SetupValue(g_Options.player_enemies_shine, "ESP", "Visible shine");
	SetupColor(g_Options.player_enemy_visible_shine, "color Visible");
	SetupColor(g_Options.color_esp_enemy_visible, "Enemies Visible");
	SetupColor(g_Options.color_esp_enemy_occluded, "Enemies Occluded");
	SetupColor(g_Options.color_chams_player_enemy_visible, "Enemy Visible ch");
	SetupColor(g_Options.color_chams_player_enemy_occluded, "Enemy Occluded ch");
	SetupColor(g_Options.color_glow_enemy, "Enemy Visible");
	SetupColor(g_Options.color_glow_enemyOC, "Enemy Occluded");
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
	SetupValue(g_Options.no_flash, "Misc", "No flash");
	SetupValue(g_Options.no_smoke, "Misc", "No smoke");
	SetupValue(g_Options.misc_bhop, "Misc", "Bunny hop");
	SetupValue(g_Options.edge_bug, "Misc", "Edge bug");
	SetupValue(g_Options.edge_bug_key, "Misc", "edge_bug_key");
	SetupValue(g_Options.edgejump.enabled, "Misc", "Edge jump");
	SetupValue(g_Options.edgejump.hotkey, "Misc", "edgejump_key");
	SetupValue(g_Options.edgejump.edge_jump_duck_in_air, "Misc", "Duck in Air");
	SetupValue(g_Options.sniper_xhair, "Misc", "Sniper crosshair");
	SetupValue(g_Options.playerModelCT, "Misc", "playerModelCT");
	SetupValue(g_Options.playerModelT, "Misc", "playerModelT");
	SetupValue(g_Options.misc_thirdperson, "Misc", "Thirdperson");
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