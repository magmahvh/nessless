#include "CLua.h"

#include "../valve_sdk/sdk.hpp"

#include <ShlObj.h>
#include <time.h>
#include "../imgui/imgui.h"
#include "../render.hpp"

#include "../features/antiaim.hpp"
#include "../helpers/keybinds.hpp"

void lua_panic(sol::optional<std::string> message) {

	Logs::Get().Create("Lua: panic state!");
	
	if (message) {
		std::string m = message.value();
		std::stringstream log;
		log << "Lua error: ";
		log << m;
		Logs::Get().Create(log.str());

		MessageBoxA(0, m.c_str(), ("Lua: panic state"), MB_APPLMODAL | MB_OK);
	}
}

std::string get_current_script(sol::this_state s)
{
	sol::state_view lua_state(s);
	sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).filename().string();

	return filename;
}

int get_current_script_id(sol::this_state s)
{
	return Lua::Get().get_script_id(get_current_script(s));
}

// ----- lua functions -----

int extract_owner(sol::this_state st) {
	sol::state_view lua_state(st);
	sol::table rs = lua_state["debug"]["getinfo"](2, "S");
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).filename().string();
	return Lua::Get().get_script_id(filename);
}

namespace ns_antiaim {
	void override_pitch(int value) {
		if (value < -89 || value > 89) {
			Logs::Get().Create("Lua: pitch is set above the allowed limits");
			return;
		}
		Antiaim::Get().pitch = value;
	}
	void override_yaw(int value) {
		if (value < -180 || value > 180) {
			Logs::Get().Create("Lua: yaw is set to more than the allowed limits");
			return;
		}
		Antiaim::Get().yaw = value;
	}
	void override_dsy(int value) {
		if (value < -59 || value > 59) {
			Logs::Get().Create("Lua: desync is set beyond the allowable limits");
			return;
		}
		Antiaim::Get().dsy = value;
	}
}

namespace ns_keybinds {
	bool get_keybinds_state(int index) {
		switch (index) {
		case 0:
			return KeyList::Get().inverter;
		case 1:
			return KeyList::Get().rollresolver;
		case 2:
			return KeyList::Get().autofire;
		default:
			Logs::Get().Create("Unknow keybinds, read documentation");
		}
	}
}

namespace ns_cheat {
	void log(std::string text) {
		Logs::Get().Create(text);
	}
	void set_event_callback(sol::this_state s, std::string eventname, sol::function func) {
		sol::state_view lua_state(s);
		sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
		std::string source = rs["source"];
		std::string filename = std::filesystem::path(source.substr(1)).filename().string();

		Lua::Get().hooks->registerHook(eventname, Lua::Get().get_script_id(filename), func);
	}
	void run_script(std::string scriptname) {
		int scrid = Lua::Get().get_script_id(scriptname);
		if (scrid < 0)
			return;

		Lua::Get().load_script(scrid);
	}

	void reload_active_scripts() {
		Lua::Get().reload_all_scripts();
	}
}

namespace ns_globals
{

	float get_realtime()
	{
		return g_GlobalVars->realtime;
	}

	float get_curtime()
	{
		return g_GlobalVars->curtime;
	}

	float get_frametime()
	{
		return g_GlobalVars->frametime;
	}

	int get_tickcount()
	{
		return g_GlobalVars->tickcount;
	}

	int get_framecount()
	{
		return g_GlobalVars->framecount;
	}

	float get_intervalpertick()
	{
		return g_GlobalVars->interval_per_tick;
	}

	int get_maxclients()
	{
		return g_GlobalVars->maxClients;
	}
}

namespace ns_entity {
	int get_player_for_user_id(int userid) {
		return g_EngineClient->GetPlayerForUserID(userid);
	}

	C_BaseEntity* get_local_player() {
		return g_LocalPlayer->GetBaseEntity();
	}
}

namespace ns_render {

	static int width, height;

	int get_screen_width()
	{
		g_EngineClient->GetScreenSize(width, height);
		return width;
	}

	int get_screen_height()
	{
		g_EngineClient->GetScreenSize(width, height);
		return height;
	}

	void draw_filled_rect(int x, int y, int x2, int y2, Color clr, float rounding = 0.f) {
		Render::Get().RenderBoxFilled(x, y, x2, y2, clr, rounding);
	}

	void draw_outlined_rect(int x, int y, int x2, int y2, Color clr, float thickness = 1.f, float rounding = 0.f) {
		Render::Get().RenderBox(x, y, x2, y2, clr, thickness, rounding);
	}

	void draw_line(int x, int y, int x2, int y2, Color clr, float thickness = 1.f) {
		Render::Get().RenderLine(x, y, x2, y2, clr, thickness);
	}

	void draw_outlined_circle(int x, int y, int radius, int segments, Color clr, float thickness = 1.f) {
		Render::Get().RenderCircle(x, y, radius, segments, clr, thickness);
	}

	void draw_filled_circle(int x, int y, int radius, int segments, Color clr) {
		Render::Get().RenderCircleFilled(x, y, radius, segments, clr);
	}

	float get_text_size(std::string text, int size) {
		return Render::Get().GetTextSize(text, size);
	}

	void draw_text(int x, int y, std::string str, int size, Color clr, bool center = false, bool outline = false) {
		Render::Get().RenderText(str, ImVec2(x, y), size, clr, center, outline);
	}
}

namespace ns_usercmd {
	int get_buttons() {
		if (!g_cmd) return 0;

		return g_cmd->buttons;
	}

	void set_buttons(int btns) {
		if (!g_cmd) return;

		g_cmd->buttons = btns;
	}
}

namespace ns_console {
	void execute(std::string& command)
	{
		if (command.empty())
			return;

		g_EngineClient->ExecuteClientCmd(command.c_str());
	}

	std::unordered_map <std::string, ConVar*> convars;

	bool get_bool(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = ("Lua error: cannot find ConVar \"") + convar_name + '\"';
				Logs::Get().Create(log);
				return false;
			}
		}

		if (!convars[convar_name])
			return false;

		return convars[convar_name]->GetBool();
	}

	int get_int(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = ("Lua error: cannot find ConVar \"") + convar_name + '\"';
				Logs::Get().Create(log);
				return 0;
			}
		}

		if (!convars[convar_name])
			return 0;

		return convars[convar_name]->GetInt();
	}

	float get_float(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = ("Lua error: cannot find ConVar \"") + convar_name + '\"';
				Logs::Get().Create(log);
				return 0.0f;
			}
		}

		if (!convars[convar_name])
			return 0.0f;

		return convars[convar_name]->GetFloat();
	}

	std::string get_string(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = ("Lua error: cannot find ConVar \"") + convar_name + '\"';
				Logs::Get().Create(log);
				return ("");
			}
		}

		if (!convars[convar_name])
			return "";

		return convars[convar_name]->GetString();
	}

	void set_bool(const std::string& convar_name, bool value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = ("Lua error: cannot find ConVar \"") + convar_name + '\"';
				Logs::Get().Create(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetBool() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_int(const std::string& convar_name, int value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = ("Lua error: cannot find ConVar \"") + convar_name + '\"';
				Logs::Get().Create(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetInt() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_float(const std::string& convar_name, float value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = ("Lua error: cannot find ConVar \"") + convar_name + '\"';
				Logs::Get().Create(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetFloat() != value) //-V550
			convars[convar_name]->SetValue(value);
	}

	void set_string(const std::string& convar_name, const std::string& value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = ("Lua error: cannot find ConVar \"") + convar_name + '\"';
				Logs::Get().Create(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetString() != value)
			convars[convar_name]->SetValue(value.c_str());
	}
}

namespace ns_menu {
	bool get_bool_value(std::string category, std::string name) {
		return g_Options.GetBoolValue(category, name);
	}

	int get_int_value(std::string category, std::string name) {
		return g_Options.GetIntValue(category, name);
	}

	int get_float_value(std::string category, std::string name) {
		return g_Options.GetFloatValue(category, name);
	}

	void add_checkbox(sol::this_state s, std::string name) {
		auto script = get_current_script(s);
		auto script_id = Lua::Get().get_script_id(script);

		Lua::Get().menu_items.push_back(item(script_id, name));
		g_Options.AddCheckbox(name);
	}

	void add_slider_int(sol::this_state s, std::string name, int min, int max) {
		auto script = get_current_script(s);
		auto script_id = Lua::Get().get_script_id(script);

		Lua::Get().menu_items.push_back(item(script_id, name));
		g_Options.AddSliderInt(name, min, max);
	}

	void add_slider_float(sol::this_state s, std::string name, float min, float max) {
		auto script = get_current_script(s);
		auto script_id = Lua::Get().get_script_id(script);

		Lua::Get().menu_items.push_back(item(script_id, name));
		g_Options.AddSliderFloat(name, min, max);
	}
}

// ----- lua functions -----

void Lua::Initialize() {
	this->lua = sol::state(sol::c_call<decltype(&lua_panic), &lua_panic>);
	this->lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::math, sol::lib::debug);

	this->lua["collectgarbage"] = sol::nil;
	this->lua["dofile"] = sol::nil;
	this->lua["load"] = sol::nil;
	this->lua["loadfile"] = sol::nil;
	this->lua["pcall"] = sol::nil;
	this->lua["print"] = sol::nil;
	this->lua["xpcall"] = sol::nil;
	this->lua["getmetatable"] = sol::nil;
	this->lua["setmetatable"] = sol::nil;
	this->lua["__nil_callback"] = [](){};

	this->lua["print"] = [](std::string s) { g_CVar->ConsolePrintf(s.c_str()); };

	lua.new_usertype <Color>(("Color"), sol::constructors <Color(), Color(int, int, int), Color(int, int, int, int)>(),
		(std::string)("r"), &Color::r,
		(std::string)("g"), &Color::g,
		(std::string)("b"), &Color::b,
		(std::string)("a"), &Color::a
		);

	lua.new_usertype <Vector>(("Vector"), sol::constructors <Vector(), Vector(float, float, float)>(),
		(std::string)("x"), &Vector::x,
		(std::string)("y"), &Vector::y,
		(std::string)("z"), &Vector::z,
		(std::string)("Length"), &Vector::Length,
		(std::string)("LengthSqr"), &Vector::LengthSqr,
		(std::string)("Length2D"), &Vector::Length2D,
		(std::string)("SetZero"), &Vector::Zero,
		(std::string)("DistTo"), &Vector::DistTo,
		(std::string)("DistToSqr"), &Vector::DistToSqr,
		(std::string)("Cross"), &Vector::Cross
		);

	lua.new_enum(("Hitboxes"),
		(std::string)("Head"), HITBOX_HEAD,
		(std::string)("Neck"), HITBOX_NECK,
		(std::string)("Pelvis"), HITBOX_PELVIS,
		(std::string)("Stomach"), HITBOX_STOMACH,
		(std::string)("Chest"), HITBOX_CHEST,
		(std::string)("Upper_Chest"), HITBOX_UPPER_CHEST,
		(std::string)("Right_Thign"), HITBOX_RIGHT_THIGH,
		(std::string)("Left_Thigh"), HITBOX_LEFT_THIGH,
		(std::string)("Right_Calf"), HITBOX_RIGHT_CALF,
		(std::string)("Left_Calf"), HITBOX_LEFT_CALF,
		(std::string)("Right_Foot"), HITBOX_RIGHT_FOOT,
		(std::string)("Left_Foot"), HITBOX_LEFT_FOOT,
		(std::string)("Right_Hand"), HITBOX_RIGHT_HAND,
		(std::string)("Left_Hand"), HITBOX_LEFT_HAND,
		(std::string)("Right_Upper_arm"), HITBOX_RIGHT_UPPER_ARM,
		(std::string)("Right_Forearm"), HITBOX_RIGHT_FOREARM,
		(std::string)("Left_Upper_arm"), HITBOX_LEFT_UPPER_ARM,
		(std::string)("Left_Forearm"), HITBOX_LEFT_FOREARM
	);

	lua.new_usertype <C_BaseCombatWeapon>(("weapon"), sol::base_classes, sol::bases<C_BaseEntity>(),
		(std::string)("CanFire"), &C_BaseCombatWeapon::CanFire,
		(std::string)("IsGun"), &C_BaseCombatWeapon::IsGun,
		(std::string)("IsGrenade"), &C_BaseCombatWeapon::IsGrenade,
		(std::string)("ItemDefinitionIndex"), &C_BaseCombatWeapon::m_iItemDefinitionIndex,
		(std::string)("NextSecondaryAttack"), &C_BaseCombatWeapon::m_flNextSecondaryAttack,
		(std::string)("PostponeFireReadyTime"), &C_BaseCombatWeapon::m_flPostponeFireReadyTime,
		(std::string)("NextPrimaryAttack"), &C_BaseCombatWeapon::m_flNextPrimaryAttack,
		(std::string)("GetSpread"), &C_BaseCombatWeapon::GetSpread,
		(std::string)("GetInaccuracy"), &C_BaseCombatWeapon::GetInaccuracy,
		(std::string)("GetWeaponData"), &C_BaseCombatWeapon::GetCSWeaponData,
		(std::string)("UpdateAccuracyPenalty"), &C_BaseCombatWeapon::UpdateAccuracyPenalty,
		(std::string)("WeaponWorldModel"), &C_BaseCombatWeapon::m_hWeaponWorldModel,
		(std::string)("ThrowTime"), &C_BaseCombatWeapon::m_fThrowTime,
		(std::string)("ViewModelIndex"), &C_BaseCombatWeapon::m_iViewModelIndex,
		(std::string)("WorldModelIndex"), &C_BaseCombatWeapon::m_iWorldModelIndex,
		(std::string)("RecoilIndex"), &C_BaseCombatWeapon::m_flRecoilIndex,
		(std::string)("Clip2"), &C_BaseCombatWeapon::m_iClip2,
		(std::string)("Clip1"), &C_BaseCombatWeapon::m_iClip1,
		(std::string)("IsKnife"), &C_BaseCombatWeapon::IsKnife,
		(std::string)("IsRifle"), &C_BaseCombatWeapon::IsRifle,
		(std::string)("IsSniper"), &C_BaseCombatWeapon::IsSniper
		);

	lua.new_usertype <C_BasePlayer>(("Player"), sol::base_classes, sol::bases<C_BaseEntity>(), //new
		(std::string)("GetIndex"), &C_BasePlayer::EntIndex,
		(std::string)("IsDormant"), &C_BasePlayer::IsDormant,
		(std::string)("GetTeam"), &C_BasePlayer::m_iTeamNum,
		(std::string)("IsAlive"), &C_BasePlayer::IsAlive,
		(std::string)("GetVelocity"), &C_BasePlayer::m_vecVelocity,
		(std::string)("GetOrigin"), &C_BasePlayer::abs_origin,
		(std::string)("GetAngles"), &C_BasePlayer::m_angEyeAngles,
		(std::string)("HasHelmet"), &C_BasePlayer::m_bHasHelmet,
		(std::string)("HasHeavyArmor"), &C_BasePlayer::m_bHasHeavyArmor,
		(std::string)("IsScoped"), &C_BasePlayer::m_bIsScoped,
		(std::string)("GetHealth"), &C_BasePlayer::m_iHealth,
		(std::string)("GetWeapon"), &C_BasePlayer::m_hActiveWeapon
		);

	lua.new_enum(("Buttons"),
		(std::string)("in_attack"), IN_ATTACK,
		(std::string)("in_jump"), IN_JUMP,
		(std::string)("in_duck"), IN_DUCK,
		(std::string)("in_forward"), IN_FORWARD,
		(std::string)("in_back"), IN_BACK,
		(std::string)("in_use"), IN_USE,
		(std::string)("in_cancel"), IN_CANCEL,
		(std::string)("in_left"), IN_LEFT,
		(std::string)("in_right"), IN_RIGHT,
		(std::string)("in_moveleft"), IN_MOVELEFT,
		(std::string)("in_moveright"), IN_MOVERIGHT,
		(std::string)("in_attack2"), IN_ATTACK2,
		(std::string)("in_run"), IN_RUN,
		(std::string)("in_reload"), IN_RELOAD,
		(std::string)("in_alt1"), IN_ALT1,
		(std::string)("in_alt2"), IN_ALT2,
		(std::string)("in_score"), IN_SCORE,
		(std::string)("in_speed"), IN_SPEED,
		(std::string)("in_walk"), IN_WALK,
		(std::string)("in_zoom"), IN_ZOOM,
		(std::string)("in_weapon1"), IN_WEAPON1,
		(std::string)("in_weapon2"), IN_WEAPON2,
		(std::string)("in_bullrush"), IN_BULLRUSH,
		(std::string)("in_grenade1"), IN_GRENADE1,
		(std::string)("in_grenade2"), IN_GRENADE2
	);

	lua.new_enum(("FrameStage"),
		(std::string)("FRAME_UNDEFINED"), FRAME_UNDEFINED,
		(std::string)("FRAME_START"), FRAME_START,
		(std::string)("FRAME_NET_UPDATE_START"), FRAME_NET_UPDATE_START,
		(std::string)("FRAME_NET_UPDATE_POSTDATAUPDATE_START"), FRAME_NET_UPDATE_POSTDATAUPDATE_START,
		(std::string)("FRAME_NET_UPDATE_POSTDATAUPDATE_END"), FRAME_NET_UPDATE_POSTDATAUPDATE_END,
		(std::string)("FRAME_NET_UPDATE_END"), FRAME_NET_UPDATE_END,
		(std::string)("FRAME_RENDER_START"), FRAME_RENDER_START,
		(std::string)("FRAME_RENDER_END"), FRAME_RENDER_END
	);

	auto antiaim = this->lua.create_table();
	antiaim["OverridePitch"] = ns_antiaim::override_pitch;
	antiaim["OverrideYaw"] = ns_antiaim::override_yaw;
	antiaim["OverrideDesyncLimit"] = ns_antiaim::override_dsy;

	auto keybinds = this->lua.create_table();
	keybinds["GetState"] = ns_keybinds::get_keybinds_state;

	auto cheat = this->lua.create_table();
	cheat["Log"] = ns_cheat::log;
	cheat["RegisterCallback"] = ns_cheat::set_event_callback;
	cheat["RunScript"] = ns_cheat::run_script;
	cheat["ReloadActiveScripts"] = ns_cheat::reload_active_scripts;

	auto globals = this->lua.create_table();
	globals["CurTime"] = ns_globals::get_curtime;
	globals["FrameCount"] = ns_globals::get_framecount;
	globals["FrameTime"] = ns_globals::get_frametime;
	globals["IntervalPerTick"] = ns_globals::get_intervalpertick;
	globals["MaxClients"] = ns_globals::get_maxclients;
	globals["RealTime"] = ns_globals::get_realtime;
	globals["TickCount"] = ns_globals::get_tickcount;

	auto entity = this->lua.create_table();
	entity["GetLocalPlayer"] = ns_entity::get_local_player;
	entity["GetPlayerForUserId"] = ns_entity::get_player_for_user_id;

	auto render = this->lua.create_table();
	render["GetScreenWidth"] = ns_render::get_screen_width;
	render["GetScreenHeight"] = ns_render::get_screen_height;
	render["Text"] = ns_render::draw_text;
	render["GetTextSize"] = ns_render::get_text_size;
	render["Rect"] = ns_render::draw_outlined_rect;
	render["FilledRect"] = ns_render::draw_filled_rect;
	render["Circle"] = ns_render::draw_outlined_circle;
	render["FilledCircle"] = ns_render::draw_filled_circle;
	render["Line"] = ns_render::draw_line;

	auto usercmd = this->lua.create_table();
	usercmd["GetButtons"] = ns_usercmd::get_buttons;
	usercmd["SetButtons"] = ns_usercmd::set_buttons;

	auto console = this->lua.create_table();
	console["Execute"] = ns_console::execute;
	console["GetBool"] = ns_console::get_bool;
	console["GetFloat"] = ns_console::get_float;
	console["GetInt"] = ns_console::get_int;
	console["GetString"] = ns_console::get_string;
	console["SetBool"] = ns_console::set_bool;
	console["SetFloat"] = ns_console::set_float;
	console["SetInt"] = ns_console::set_int;
	console["SetString"] = ns_console::set_string;

	auto menu = this->lua.create_table();
	menu["GetBoolValue"] = ns_menu::get_bool_value;
	menu["GetIntValue"] = ns_menu::get_int_value;
	menu["GetFloatValue"] = ns_menu::get_float_value;
	menu["AddCheckbox"] = ns_menu::add_checkbox;
	menu["AddSliderInt"] = ns_menu::add_slider_int;
	menu["AddSliderFloat"] = ns_menu::add_slider_float;

	this->lua["AntiAim"] = antiaim;
	this->lua["KeyBinds"] = keybinds;
	this->lua["Cheat"] = cheat;
	this->lua["Globals"] = globals;
	this->lua["Entity"] = entity;
	this->lua["Render"] = render;
	this->lua["UserCmd"] = usercmd;
	this->lua["Console"] = console;
	this->lua["Menu"] = menu;

	this->refresh_scripts();
	//this->load_script(this->get_script_id("autorun.lua"));
}

void Lua::load_script(int id) {
	if (id == -1)
		return;

	if (this->loaded.at(id))
		return;

	auto path = this->get_script_path(id);
	if (path ==  (""))
		return;

	this->lua.script_file(path, [](lua_State*, sol::protected_function_result result) {
		if (!result.valid()) {
			sol::error err = result;
			Logs::Get().Create(err.what());
		}

		return result;
		});

	this->loaded.at(id) = true;
}

void Lua::unload_script(int id) {
	if (id == -1)
		return;

	if (!this->loaded.at(id))
		return;

	for (auto i : Lua::Get().menu_items) {
		if (i.id == id) {
			for (auto value : g_Options.bools) {
				if (value->category == "profile.lua_elements") {
					if (value->name == i.name) {
						g_Options.bools.erase(std::remove(g_Options.bools.begin(), g_Options.bools.end(), value), g_Options.bools.end());
					}
				}
			}
			for (auto value : g_Options.ints) {
				if (value->category == "profile.lua_elements") {
					if (value->name == i.name) {
						g_Options.ints.erase(std::remove(g_Options.ints.begin(), g_Options.ints.end(), value), g_Options.ints.end());
					}
				}
			}
			for (auto value : g_Options.floats) {
				if (value->category == "profile.lua_elements") {
					if (value->name == i.name) {
						g_Options.floats.erase(std::remove(g_Options.floats.begin(), g_Options.floats.end(), value), g_Options.floats.end());
					}
				}
			}
		}
	}


	hooks->unregisterHooks(id);
	this->loaded.at(id) = false;
}

void Lua::reload_all_scripts() {
	for (auto s : this->scripts) {
		if (this->loaded.at(this->get_script_id(s))) {
			this->unload_script(this->get_script_id(s));
			this->load_script(this->get_script_id(s));
		}
	}
}

void Lua::unload_all_scripts() {
	for (auto s : this->scripts)
		if (this->loaded.at(this->get_script_id(s)))
			this->unload_script(this->get_script_id(s));
}

void Lua::refresh_scripts() {
	auto oldLoaded = this->loaded;
	auto oldScripts = this->scripts;

	this->loaded.clear();
	this->pathes.clear();
	this->scripts.clear();

	CHAR my_documents[MAX_PATH];
	SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, my_documents);

	for (auto& entry : std::filesystem::directory_iterator(my_documents + std::string("\\desolate\\lua\\"))) {
		if (entry.path().extension() == (".lua")) {
			auto path = entry.path();
			auto filename = path.filename().string();

			bool didPut = false;
			for (int i = 0; i < oldScripts.size(); i++) {
				if (filename == oldScripts.at(i)) {
					this->loaded.push_back(oldLoaded.at(i));
					didPut = true;
				}
			}

			if (!didPut)
				this->loaded.push_back(false);

			this->pathes.push_back(path);
			this->scripts.push_back(filename);
		}
	}
}

int Lua::get_script_id(std::string name) {
	for (int i = 0; i < this->scripts.size(); i++) {
		if (this->scripts.at(i) == name)
			return i;
	}

	return -1;
}

int Lua::get_script_id_by_path(std::string path) {
	for (int i = 0; i < this->pathes.size(); i++) {
		if (this->pathes.at(i).string() == path)
			return i;
	}

	return -1;
}

std::string Lua::get_script_path(std::string name) {
	return this->get_script_path(this->get_script_id(name));
}

std::string Lua::get_script_path(int id) {
	if (id == -1)
		return  "";

	return this->pathes.at(id).string();
}
