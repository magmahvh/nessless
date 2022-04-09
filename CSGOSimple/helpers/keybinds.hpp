#pragma once

#include "..//options.hpp"
#include "..//valve_sdk/csgostructs.hpp"
#include "../lua/CLua.h"
#include "../ui.hpp"
#include "../options.hpp"

class KeyBinds : public Singleton<KeyBinds>
{
public:
	bool GetHotkeyActive(std::string name, int key, int type);
	void DrawKeyBind(const char* label, int* key, int* type);
	void BindsSet();
	void Initialize();

	inline static std::unordered_map<std::string, bool> m_hotkey_states{};
};

class KeyList : public Singleton<KeyList>
{
public:
	bool rollresolver = false;
	bool inverter = false;
	bool autofire = false;
};