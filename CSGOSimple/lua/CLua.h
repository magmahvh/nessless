#pragma once
#pragma comment(lib, "lua53.lib")

#include "../singleton.hpp"
#include "../helpers/logs.hpp"
#include "CLuaHook.h"

#include <filesystem>

struct item
{
	int id;
	std::string name;
	item(int id_, std::string name_) :
		id(id_), name(name_)
	{

	}

};

class Lua : public Singleton<Lua> {
public:
	void refresh_scripts();

	void Initialize();

	void load_script(int id);
	void unload_script(int id);

	void reload_all_scripts();
	void unload_all_scripts();

	int get_script_id(std::string name);
	int get_script_id_by_path(std::string path);

	std::vector<bool> loaded;
	std::vector<std::string> scripts;

	c_lua_hookManager* hooks = new c_lua_hookManager();
	std::vector<item> menu_items = {};

	sol::state lua;

private:
	std::string get_script_path(std::string name);
	std::string get_script_path(int id);

	std::vector<std::filesystem::path> pathes;
};
