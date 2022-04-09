#include "keybinds.hpp"

bool KeyBinds::GetHotkeyActive(std::string name, int key, int type) {
	switch (type) {
	case 0:
		m_hotkey_states[name] = GetKeyState(key);
		return m_hotkey_states[name];
		break;
	case 1:
		return GetAsyncKeyState(key);
		break;
	case 2:
		return !GetAsyncKeyState(key);
		break;
	case 3:
		return true;
		break;
	}

	return false;
}

void KeyBinds::DrawKeyBind(const char* label, int* key, int* type) {
	ImGui::Combo(label, type, "Toggle\0Hold ON\0Hold OFF\0Always ON");
	ImGui::SameLine();
	ImGui::Hotkey("key_" + *label, key);
}

void KeyBinds::BindsSet() {
	KeyList::Get().rollresolver = KeyBinds::Get().GetHotkeyActive("rollresolver", g_Options.roll_resolver_key, g_Options.roll_resolver_type);
	KeyList::Get().inverter = KeyBinds::Get().GetHotkeyActive("inverter", g_Options.antiaim_flip_key, g_Options.antiaim_flip_type);
	KeyList::Get().autofire = KeyBinds::Get().GetHotkeyActive("autofire", g_Options.autofire_key, g_Options.autofire_type);
}