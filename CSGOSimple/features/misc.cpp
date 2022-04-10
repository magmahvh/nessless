#include "misc.hpp"

#include "../helpers/math.hpp"
#include <algorithm>
#include "../helpers/keybinds.hpp"

void Misc::ThirdPerson() {
	if (!g_Options.misc_thirdperson) return;
	if (!g_LocalPlayer) return;
	if (!g_LocalPlayer->IsAlive()) return;

	static bool is_active = false;

	if (KeyList::Get().thirdperson) {
		if (is_active) return;

		is_active = true;
		ConVar* sv_cheats = g_CVar->FindVar("sv_cheats");
		*(int*)((DWORD)&sv_cheats->m_fnChangeCallbacks + 0xC) = 0; // ew
		sv_cheats->SetValue(1);
		g_EngineClient->ExecuteClientCmd("thirdperson");
	}
	else {
		if (!is_active) return;

		is_active = false;
		ConVar* sv_cheats = g_CVar->FindVar("sv_cheats");
		*(int*)((DWORD)&sv_cheats->m_fnChangeCallbacks + 0xC) = 0; // ew
		sv_cheats->SetValue(1);
		g_EngineClient->ExecuteClientCmd("firstperson");
	}
}

