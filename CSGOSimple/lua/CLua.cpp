//#include "CLua.h"
//
//#include "../valve_sdk/sdk.hpp"
//
//#include <ShlObj.h>
//#include <time.h>
//
//void lua_panic(sol::optional<std::string> message) {
//
//	Logs::Get().Create("Lua: panic state!");
//	
//	if (message) {
//		std::string m = message.value();
//		std::stringstream log;
//		log << "Lua error: ";
//		log << m;
//		Logs::Get().Create(log.str());
//
//		MessageBoxA(0, m.c_str(), ("Lua: panic state"), MB_APPLMODAL | MB_OK);
//	}
//}
//
//// ----- lua functions -----
//
//int extract_owner(sol::this_state st) {
//	sol::state_view lua_state(st);
//	sol::table rs = lua_state["debug"]["getinfo"](2, "S");
//	std::string source = rs["source"];
//	std::string filename = std::filesystem::path(source.substr(1)).filename().string();
//	return Lua::Get().get_script_id(filename);
//}
//
//namespace ns_logs {
//	void create(std::string text) {
//		Logs::Get().Create(text);
//	}
//}
//
//// ----- lua functions -----
//
//void Lua::Initialize() {
//	this->lua = sol::state(sol::c_call<decltype(&lua_panic), &lua_panic>);
//	this->lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::math, sol::lib::debug);
//
//	this->lua["collectgarbage"] = sol::nil;
//	this->lua["dofile"] = sol::nil;
//	this->lua["load"] = sol::nil;
//	this->lua["loadfile"] = sol::nil;
//	this->lua["pcall"] = sol::nil;
//	this->lua["print"] = sol::nil;
//	this->lua["xpcall"] = sol::nil;
//	this->lua["getmetatable"] = sol::nil;
//	this->lua["setmetatable"] = sol::nil;
//	this->lua["__nil_callback"] = [](){};
//
//	this->lua["print"] = [](std::string s) { g_CVar->ConsolePrintf(s.c_str()); };
//
//	auto logs = this->lua.create_table();
//	logs["Create"] = ns_logs::create;
//
//	this->lua["Logs"] = logs;
//
//	this->refresh_scripts();
//	this->load_script(this->get_script_id("autorun.lua"));
//}
//
//void Lua::load_script(int id) {
//	if (id == -1)
//		return;
//
//	if (this->loaded.at(id))
//		return;
//
//	auto path = this->get_script_path(id);
//	if (path ==  (""))
//		return;
//
//	this->lua.script_file(path, [](lua_State*, sol::protected_function_result result) {
//		if (!result.valid()) {
//			sol::error err = result;
//			Logs::Get().Create(err.what());
//		}
//
//		return result;
//		});
//
//	this->loaded.at(id) = true;
//}
//
//void Lua::unload_script(int id) {
//	if (id == -1)
//		return;
//
//	if (!this->loaded.at(id))
//		return;
//
//	std::map<std::string, std::map<std::string, std::vector<MenuItem_t>>> updated_items;
//	for (auto i : this->menu_items) {
//		for (auto k : i.second) {
//			std::vector<MenuItem_t> updated_vec;
//
//			for (auto m : k.second)
//				if (m.script != id)
//					updated_vec.push_back(m);
//
//			updated_items[k.first][i.first] = updated_vec;
//		}
//	}
//	this->menu_items = updated_items;
//
//	hooks->unregisterHooks(id);
//	this->loaded.at(id) = false;
//}
//
//void Lua::reload_all_scripts() {
//	for (auto s : this->scripts) {
//		if (this->loaded.at(this->get_script_id(s))) {
//			this->unload_script(this->get_script_id(s));
//			this->load_script(this->get_script_id(s));
//		}
//	}
//}
//
//void Lua::unload_all_scripts() {
//	for (auto s : this->scripts)
//		if (this->loaded.at(this->get_script_id(s)))
//			this->unload_script(this->get_script_id(s));
//}
//
//void Lua::refresh_scripts() {
//	auto oldLoaded = this->loaded;
//	auto oldScripts = this->scripts;
//
//	this->loaded.clear();
//	this->pathes.clear();
//	this->scripts.clear();
//
//	CHAR my_documents[MAX_PATH];
//	SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, my_documents);
//
//	for (auto& entry : std::filesystem::directory_iterator(my_documents + std::string("\\nessless\\lua\\"))) {
//		if (entry.path().extension() == (".lua")) {
//			auto path = entry.path();
//			auto filename = path.filename().string();
//
//			bool didPut = false;
//			for (int i = 0; i < oldScripts.size(); i++) {
//				if (filename == oldScripts.at(i)) {
//					this->loaded.push_back(oldLoaded.at(i));
//					didPut = true;
//				}
//			}
//
//			if (!didPut)
//				this->loaded.push_back(false);
//
//			this->pathes.push_back(path);
//			this->scripts.push_back(filename);
//		}
//	}
//}
//
//int Lua::get_script_id(std::string name) {
//	for (int i = 0; i < this->scripts.size(); i++) {
//		if (this->scripts.at(i) == name)
//			return i;
//	}
//
//	return -1;
//}
//
//int Lua::get_script_id_by_path(std::string path) {
//	for (int i = 0; i < this->pathes.size(); i++) {
//		if (this->pathes.at(i).string() == path)
//			return i;
//	}
//
//	return -1;
//}
//
//std::string Lua::get_script_path(std::string name) {
//	return this->get_script_path(this->get_script_id(name));
//}
//
//std::string Lua::get_script_path(int id) {
//	if (id == -1)
//		return  "";
//
//	return this->pathes.at(id).string();
//}
