#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include "valve_sdk/Misc/Color.hpp"

#define A( s ) #s
#define OPTION(type, var, val) Var<type> var = {A(var), val}

template <typename T>
class ConfigValue
{
public:
	ConfigValue(std::string category_, std::string name_, T* value_, T min_ = 0, T max_ = 0)
	{
		category = category_;
		name = name_;
		value = value_;
		min = min_;
		max = max_;
	}

	std::string category, name;
	T* value;
	T min, max;
};
struct statrack_setting
{
	int definition_index = 1;
	struct
	{
		int counter = 0;
	}statrack_new;
};
struct item_setting
{
	char name[32] = "Default";
	//bool enabled = false;
	int stickers_place = 0;
	int definition_vector_index = 0;
	int definition_index = 0;
	bool   enabled_stickers = 0;
	int paint_kit_vector_index = 0;
	int paint_kit_index = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	bool stat_trak = 0;
	float wear = FLT_MIN;
	char custom_name[32] = "";
};

class Options
{
public:
	struct
	{
		/*struct
		{
			std::map<int, profilechanger_settings> profile_items = { };
		}profile;*/
		struct
		{
			bool skin_preview = false;
			bool show_cur = true;

			std::map<int, statrack_setting> statrack_items = { };
			std::map<int, item_setting> m_items = { };
			std::map<std::string, std::string> m_icon_overrides = { };
		}skin;
	}changers;

	int legitbot_weapon = 0;
	bool legit_enabled = false;
	struct
	{
		bool enabled = false;
		bool deathmatch = false;
		bool silent = false;

		bool flash_check = false;
		bool smoke_check = false;
		bool autopistol = false;

		float fov = 0.f;
		float silent_fov = 0.f;
		float smooth = 1.f;

		int shot_delay = 0;
		int kill_delay = 0;

		struct
		{
			bool head = true;
			bool chest = true;
			bool hands = true;
			bool legs = true;
		} hitboxes;

		struct
		{
			bool enabled = false;
			int start = 1;
			int x = 100;
			int y = 100;
		} rcs;

		struct
		{
			bool enabled = false;
			int min_damage = 1;
		} autowall;
	} legitbot[5];
	
	bool autofire_enabled = false;
	int autofire_key = 0;
	int autofire_type = 0;

	int ragebot_weapon = 0;
	bool rage_enabled = false;
	
	int roll_resolver_key = 0;
	int roll_resolver_type = 0;
	struct
	{
		bool enabled = false;
		bool autoshot = false;
		bool autowall = false;
		bool autostop = false;
		bool silent = false;

		int damage = 1;
		int hitchance = 1;

		int multipoint_body = 1;
		int multipoint_head = 1;

		struct
		{
			bool head = true;
			bool upper_chest = true;
			bool chest = true;
			bool lower_chest = true;
			bool hands = true;
			bool legs = true;
		} hitboxes;
	} ragebot[6];

	
	bool antiaim = false;
	int antiaim_pitch = 0;
	int antiaim_yaw = 0;
	int antiaim_dsy = 1;

	int antiaim_flip_key = 0;
	int antiaim_flip_type = 0;

	bool fakelag = false;
	int fakelag_amount = 1;


	// 
	// ESP
	// 

	bool InThirdPerson = false;
	float OFOV = 0;
	int SHeight = 0;
	int SWidth = 0;
	float SWidthHalf = 0;
	float SHeightHalf = 0;
	bool change_materials = false;

	bool esp_enemies_only = false;
	bool esp_player_boxes = false;
	bool esp_player_boxesOccluded = false;
	bool esp_player_names = false;
	bool esp_player_health = false;
	bool esp_player_armour = false;
	bool esp_player_weapons = false;
	bool esp_player_snaplines = false;
	bool esp_dropped_weapons = false;
	bool esp_defuse_kit = false;
	bool esp_planted_c4 = false;
	bool esp_items = false;

	float esp_nightmode_size = 0.05f;
	float fog_start_distance = 0.0f;
	float fog_end_distance = 0.0f;
	float fog_density = 0.0f;
	bool enable_fog = false;

	bool remove_smoke = false;
	bool remove_scope = false;
	bool remove_visualrecoil = false;
	bool remove_post_processing = false;
	bool remove_zoom = false;
	bool remove_flash = false;

	// 
	// GLOW
	// 
	bool glow_enabled = false;
	bool glow_enemies_only = false;
	bool glow_players = false;
	bool glow_chickens = false;
	bool glow_c4_carrier = false;
	bool glow_planted_c4 = false;
	bool glow_defuse_kits = false;
	bool glow_weapons = false;
	bool glow_enemiesOC = false;

	//
	// CHAMS
	//
	bool chams_player_enabled = false;
	bool player_enemies_shine = false;
	bool player_enemies_flat = false;


	bool chams_player_enemies_only = false;
	bool chams_player_wireframe = false;
	int chams_player_flat = false;
	bool chams_player_ignorez = false;
	bool chams_player_glass = false;
	bool chams_arms_enabled = false;
	bool chams_arms_wireframe = false;
	bool chams_arms_flat = false;
	bool chams_arms_ignorez = false;
	bool chams_arms_glass = false;

	//
	// MISC
	//
	bool autowalk = false;

	bool misc_autostrafe = false;
	bool misc_boostspeed = false;
	bool misc_wasdstrafes = false;
	bool misc_bhop = false;
	float misc_retrack_speed = 2.0f;
	int playerModelT{ 0 };
	int playerModelCT{ 0 };

	bool logs = true;
	bool logs_drawing = true;

	int misc_combo_clantag = 0;

	bool misc_showranks = false;
	bool misc_watermark = false;
	bool Velocity = false;
	bool outline = false;
	bool lastjump = false;
	bool lastjumpoutline = false;
	bool autoaccept = false;
	bool enable_offsets = false;
	float viewmodel_offset_x = 0.0f;
	float viewmodel_offset_y = 0.0f;
	float viewmodel_offset_z = 0.0f;
	float viewmodel_offset_roll = 0.0f;
	bool enable_post_proc = false;
	int  viewmodel_fov = 68;
	float asus_props = 0.0f;
	float asus_walls = 0.0f;
	bool aspect_ratio = false;
	int skybox_num = false;
	float aspect_ratio_scale = 1.0f;
	bool enable_nightmode = false;
	bool no_scope_crosshair = false;
	int misc_thirdperson_bind = 0;
	float misc_thirdperson_dist = 50.f;

	struct
	{
		bool enabled = false;
		bool edge_jump_duck_in_air = false;

		int hotkey = 0;
	} edgejump;
	bool edge_bug;
	int edge_bug_key;
	int glow_enemies_type;
	bool sniper_xhair = false;

	// 
	// COLORS
	// 
	Color color_esp_ally_visible = { 0, 0, 0 };
	Color color_esp_enemy_visible = { 0, 0, 0 };
	Color color_esp_ally_occluded = { 0, 0, 0 };
	Color color_esp_enemy_occluded = { 0, 0, 0 };
	Color color_esp_weapons = { 0, 0, 0 };
	Color color_esp_defuse = { 0, 0, 0 };
	Color color_esp_c4 = { 0, 0, 0 };
	Color color_esp_item = { 0, 0, 0 };
	Color Velocitycol = { 0, 0, 0 };

	Color color_glow_ally = { 0, 0, 0 };
	Color color_glow_allyOC = { 0, 0, 0 };

	Color color_glow_enemy = { 0, 0, 0 };
	Color color_glow_enemyOC = { 0, 0, 0 };

	Color color_glow_chickens = { 0, 0, 0 };
	Color color_glow_c4_carrier = { 0, 0, 0 };
	Color color_glow_planted_c4 = { 0, 0, 0 };
	Color color_glow_defuse = { 0, 0, 0 };
	Color color_glow_weapons = { 0, 0, 0 };

	Color color_chams_player_ally_visible = { 0, 0, 0 };
	Color color_chams_player_ally_occluded = { 0, 0, 0 };
	Color color_chams_player_enemy_visible = { 0, 0, 0 };
	Color color_chams_player_enemy_occluded = { 0, 0, 0 };
	Color color_chams_arms_visible = { 0, 0, 0 };
	Color color_chams_arms_occluded = { 0, 0, 0 };
	Color color_watermark = { 0, 0, 0 }; // no menu config cuz its useless
	Color player_enemy_visible_shine = { 0, 0, 0 };
	Color player_enemy_flat = { 0, 0, 0 };
	Color nightmode_color = { 255, 255, 255 };
	Color skybox_color = { 255, 255, 255 };
	Color fog_color = { 255, 255, 255 };
	Color post_processing = { 0, 0, 0 };

	Color menu_color = { 100, 120, 235 };
private:
	//	void SetupValue(char value, std::string category, std::string name);
	void SetupValue(int& value, std::string category, std::string name);
	void SetupValue(bool& value, std::string category, std::string name);
	void SetupValue(float& value, std::string category, std::string name);
	void SetupColor(Color& value, const std::string& name);
	void SetupWeapons();
	void SetupVisuals();
	void SetupMisc();
	void SetupColors();
public:
	inline static std::unordered_map<std::string, bool> bool_elements{};
	inline static std::unordered_map<std::string, int> int_elements{};
	inline static std::unordered_map<std::string, float> float_elements{};

	std::vector<ConfigValue<int>*> ints;
	std::vector<ConfigValue<bool>*> bools;
	std::vector<ConfigValue<float>*> floats;

	void Initialize();
	void LoadSettings(const std::string& szIniFile);
	void SaveSettings(const std::string& szIniFile);
	void DeleteSettings(const std::string& szIniFile);

	int GetIntValue(std::string category, std::string name);
	int GetBoolValue(std::string category, std::string name);
	int GetFloatValue(std::string category, std::string name);

	void AddCheckbox(std::string name);
	void AddSliderInt(std::string name, int min, int max);
	void AddSliderFloat(std::string name, float min, float max);

	std::string folder;
};

inline Options g_Options;
inline bool   g_Unload;