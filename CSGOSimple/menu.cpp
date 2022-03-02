#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"
#include "features/item_definitions.h"
#include "features/kit_parser.h"
#include "features/skins.h"
#include "render.hpp"

const char* legit_weapons = "Pistols\0Rifles\0Deagle\0Sniper\0Other";
const char* rage_weapons = "AWP\0Auto-Sniper\0Scout\0Deagle and R8\0Pistols\0Other";

void ReadDirectory(const std::string& name, std::vector<std::string>& v)
{
	auto pattern(name);
	pattern.append("\\*.ini");
	WIN32_FIND_DATAA data;
	HANDLE hFind;
	if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			v.emplace_back(data.cFileName);
		} while (FindNextFileA(hFind, &data) != 0);
		FindClose(hFind);
	}
}
struct hud_weapons_t {
	std::int32_t* get_weapon_count() {
		return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
	}
};

template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(Utils::PatternScan2("client.dll", "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utils::PatternScan2("client.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)find_hud_element(pThis, name);
}
namespace ImGuiEx
{
	inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
	{
		float clr[4] = {
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};
		//clr[3]=255;
		if (ImGui::ColorEdit4(label, clr, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar)) {
			v->SetColor(clr[0], clr[1], clr[2], clr[3]);
			return true;
		}
		return false;
	}
	inline bool ColorEdit4a(const char* label, Color* v, bool show_alpha = true)
	{
		float clr[4] = {
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};
		//clr[3]=255;
		if (ImGui::ColorEdit4(label, clr, show_alpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar)) {
			v->SetColor(clr[0], clr[1], clr[2], clr[3]);
			return true;
		}
		return false;
	}

	inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

void Menu::Initialize()
{
	CreateStyle();

    _visible = true;
}

void Menu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}

bool Tab(const char* label, const ImVec2& size_arg, bool state)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);
	if (pressed)
		ImGui::MarkItemEdited(id);

	static std::map<ImGuiID, int> alpha_anim;
	auto it_alpha = alpha_anim.find(id);
	if (it_alpha == alpha_anim.end())
	{
		alpha_anim.insert({ id, 0 });
		it_alpha = alpha_anim.find(id);
	}
	if (state) {
		if (it_alpha->second < 250)
			it_alpha->second += 10;
	} else {
		if (it_alpha->second > 0) 
			it_alpha->second -= 10; 
	}

	static std::map<ImGuiID, int> alpha_anim2;
	auto it_alpha2 = alpha_anim2.find(id);
	if (it_alpha2 == alpha_anim2.end())
	{
		alpha_anim2.insert({ id, 0 });
		it_alpha2 = alpha_anim2.find(id);
	}
	if (state) {
		if (it_alpha2->second < size_arg.x)
			it_alpha2->second += 2;
	}
	else {
		if (it_alpha2->second > 0)
			it_alpha2->second -= 2;
	}

	if (it_alpha2->second > 0 || it_alpha->second > 0)
	{
		window->DrawList->AddRectFilled(bb.Min + ImVec2(size_arg.x / 2 - it_alpha2->second / 2, 0), bb.Min + ImVec2(it_alpha2->second, 4), ImColor(100, 120, 235, it_alpha->second), 2);
		window->DrawList->AddRectFilled(bb.Min + ImVec2(0, 2), bb.Max, ImColor(25, 25, 25, it_alpha->second));
		window->DrawList->AddRectFilled(bb.Min + ImVec2(1, 2), bb.Max - ImVec2(1, 0), ImColor(11, 11, 11, it_alpha->second));
	}

	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	return pressed;
}
bool subTab(const char* label, const ImVec2& size_arg, bool state)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);
	if (pressed)
		ImGui::MarkItemEdited(id);

	static std::map<ImGuiID, int> alpha_anim;
	auto it_alpha = alpha_anim.find(id);
	if (it_alpha == alpha_anim.end())
	{
		alpha_anim.insert({ id, 0 });
		it_alpha = alpha_anim.find(id);
	}
	if (state) {
		if (it_alpha->second < 80)
			it_alpha->second += 4;
	}
	else {
		if (it_alpha->second > 0)
			it_alpha->second -= 4;
	}


	window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max, ImColor(40, 40, 40, 80 - it_alpha->second), ImColor(40, 40, 40, 0), ImColor(40, 40, 40, 0), ImColor(40, 40, 40, 80 - it_alpha->second));
	window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max, ImColor(86, 93, 200, it_alpha->second), ImColor(86, 93, 200, 0), ImColor(86, 93, 200, 0), ImColor(86, 93, 200, it_alpha->second));
	window->DrawList->AddRectFilled(bb.Min, bb.Min + ImVec2(3, size_arg.y), ImColor(86, 93, 200, it_alpha->second * 3));
	window->DrawList->AddText(bb.Min + ImVec2(20, size_arg.y / 2 - ImGui::CalcTextSize(label).y / 2), ImColor(220, 220, 220), label);

	return pressed;
}
void Menu::SpectatorList()
{
	if (!g_Options.spectator_list)
		return;

	std::string spectators;

	if (g_EngineClient->IsInGame() && g_LocalPlayer)
	{
		for (int i = 1; i <= g_GlobalVars->maxClients; i++)
		{
			auto ent = C_BasePlayer::GetPlayerByIndex(i);

			if (!ent || ent->IsAlive() || ent->IsDormant())
				continue;

			auto target = (C_BasePlayer*)ent->m_hObserverTarget();

			if (!target || target != g_LocalPlayer)
				continue;

			if (ent == target)
				continue;

			auto info = ent->GetPlayerInfo();

			spectators += std::string(info.szName) + u8"\n";
		}
	}

	if (ImGui::Begin("Spectator List", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground |( _visible ? NULL : ImGuiWindowFlags_NoMove)))
	{
		ImGui::PushFont(g_MenuFont);

		ImGui::Text("Spectator List");

		ImGui::Text(spectators.c_str());
		ImGui::PopFont();

	}
	ImGui::End();
}

void Menu::Render()
{
	ImGui::GetIO().MouseDrawCursor = _visible;
	SpectatorList();
    if(!_visible)
        return;
	auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | NULL | NULL | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | NULL | NULL | NULL;

	ImGuiStyle* Style = &ImGui::GetStyle();
	static int tab = 0;
	static float x = 600, y = 400;
	static char cfg_name[64] = { 0 };
	static char cfg_name_new[64] = { 0 };
	const char* tabs[] = {
		"Rage", "Legit", "Visuals", "Misc", "Profile", "Scripting"
	};
	static int subtab[] = {
		0, 0, 0, 0, 0, 0
	};

	const char* subtabs_rage[] = {
		"General", "Weapon"
	};
	const char* subtabs_legit[] = {
		"General", "Other"
	}; 
	const char* subtabs_visuals[] = {
		"General"
	}; 
	const char* subtabs_misc[] = {
		"General"
	}; 
	const char* subtabs_profile[] = {
		"Configs", "Scripts"
	}; 
	const char* subtabs_scripting[] = {
		"Items"
	};

	static int selected = 0;
	static char cfgName[64];
	std::vector<std::string> cfgList;

	Style->Colors[ImGuiCol_Text] = ImColor(220, 220, 220);
	Style->Colors[ImGuiCol_WindowBg] = ImColor(11, 11, 11);
	Style->Colors[ImGuiCol_ChildBg] = ImColor(7, 7, 7);
	Style->WindowBorderSize = 0;
	Style->WindowRounding = 0;
	Style->ChildRounding = 0;

	ImGui::SetNextWindowSize({ x, y });

	ImGui::PushFont(g_MenuFont);
	ImGui::Begin("nessless", nullptr, flags);
	{
		ImVec2 w = ImGui::GetWindowPos();
		ImVec2 p = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2{ 0, 0 });
		ImGui::BeginChild("##Tabs", ImVec2{ x, 25 });
		{
			ImGui::SetCursorPos(ImVec2(5, 4 ));
			Style->Colors[ImGuiCol_Text] = ImColor(62, 123, 234);
			ImGui::Text("nessless");
#if _DEBUG
			ImGui::SameLine();
			ImGui::Text("| alpha");
#endif
			ImGui::GetWindowDrawList()->AddLine(ImVec2(w.x, w.y + 24), ImVec2(w.x + x, w.y + 24), ImColor(25, 25, 25), 2);
			Style->Colors[ImGuiCol_Text] = ImColor(220, 220, 220);

			for (int i = 0; i < 6; i++) {
				float xpos = 160 + 70 * i;
				ImGui::SetCursorPos(ImVec2( xpos, 2 ));
				if (Tab(tabs[i], ImVec2( 70, 23 ), tab == i))
					tab = i;
			}
		}
		ImGui::EndChild();

		Style->Colors[ImGuiCol_Border] = ImColor(25, 25, 25);
		Style->ChildBorderSize = 2;

		ImGui::SetCursorPos(ImVec2{ 10, 40 });
		ImGui::BeginChild("##SubTab", ImVec2{ 120, y - 50 }, true);
		{
			switch (tab) {
			case 0:
				for (int i = 0; i < 2; i++) {
					ImGui::SetCursorPos(ImVec2(0, 25 * i));
					if (subTab(subtabs_rage[i], ImVec2(120, 25), subtab[0] == i))
						subtab[0] = i;
				}
				break;
			case 1:
				for (int i = 0; i < 2; i++) {
					ImGui::SetCursorPos(ImVec2(0, 25 * i));
					if (subTab(subtabs_legit[i], ImVec2(120, 25), subtab[1] == i))
						subtab[1] = i;
				}
				break;
			case 2:
				for (int i = 0; i < 1; i++) {
					ImGui::SetCursorPos(ImVec2(0, 25 * i));
					if (subTab(subtabs_visuals[i], ImVec2(120, 25), subtab[2] == i))
						subtab[2] = i;
				}
				break;
			case 3:
				for (int i = 0; i < 1; i++) {
					ImGui::SetCursorPos(ImVec2(0, 25 * i));
					if (subTab(subtabs_misc[i], ImVec2(120, 25), subtab[3] == i))
						subtab[3] = i;
				}
				break;
			case 4:
				for (int i = 0; i < 2; i++) {
					ImGui::SetCursorPos(ImVec2(0, 25 * i));
					if (subTab(subtabs_profile[i], ImVec2(120, 25), subtab[4] == i))
						subtab[4] = i;
				}
				break;
			case 5:
				for (int i = 0; i < 1; i++) {
					ImGui::SetCursorPos(ImVec2(0, 25 * i));
					if (subTab(subtabs_scripting[i], ImVec2(120, 25), subtab[5] == i))
						subtab[5] = i;
				}
				break;
			}
		}
		ImGui::EndChild();

		int last_x = x - (10 + 120 + 30);

		auto legitbot = &g_Options.legitbot[g_Options.legitbot_weapon];
		auto ragebot = &g_Options.ragebot[g_Options.ragebot_weapon];

		ImGui::SetCursorPos(ImVec2{ 10 + 120 + 10, 40 });
		ImGui::BeginChild("##FuncitonalTab1", ImVec2{ (float)(last_x / 2), y - 85 }, true);
		{
			switch (tab) {
			case 0:
				switch (subtab[0]) {
				case 0:
					ImGui::Separator("General");
					ImGui::Checkbox("Enable Ragebot", &g_Options.rage_enabled);
					if (g_Options.rage_enabled) {
						g_Options.legit_enabled = false;
					}
					break;
				case 1:
					ImGui::Separator("Weapons");
					ImGui::Combo("Weapon", &g_Options.ragebot_weapon, rage_weapons);
					ImGui::Checkbox("Enabled Weapon", &ragebot->enabled);
					ImGui::Checkbox("Auto shot", &ragebot->autoshot);
					ImGui::Checkbox("Through obstacle", &ragebot->autowall); 
					ImGui::Checkbox("Silent", &ragebot->silent);
					break;
				}
				break;
			case 1:
				switch (subtab[1]) {
				case 0:
					ImGui::Separator("Weapons");
					ImGui::Checkbox("Enabled Legitbot", &g_Options.legit_enabled);
					if (g_Options.legit_enabled) {
						g_Options.rage_enabled = false;
					}
					ImGui::Combo("Weapon", &g_Options.legitbot_weapon, legit_weapons);
					ImGui::Checkbox("Enabled Weapon", &legitbot->enabled);
					//ImGui::Checkbox("Friendly fire", &settings->deathmatch);
					ImGui::Checkbox("Silent", &legitbot->silent);
					ImGui::Checkbox("Flash check", &legitbot->flash_check);
					ImGui::Checkbox("Smoke check", &legitbot->smoke_check);
					ImGui::Checkbox("Auto-pistol", &legitbot->autopistol);

					if (ImGui::BeginCombo("##hitbox_filter", "Hitboxes", ImGuiComboFlags_NoArrowButton))
					{
						ImGui::Selectable("Head", &legitbot->hitboxes.head, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Chest", &legitbot->hitboxes.chest, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Hands", &legitbot->hitboxes.hands, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Legs", &legitbot->hitboxes.legs, ImGuiSelectableFlags_DontClosePopups);

						ImGui::EndCombo();
					}
					break;
				case 1:
					ImGui::Checkbox("Enabled autofire##autofire", &legitbot->autofire.enabled);
					ImGui::SameLine();
					ImGui::Hotkey("##autofire", &legitbot->autofire.hotkey);
					break;
				}
				break;
			case 2:
				switch (subtab[2]) {
				case 0:
					ImGui::Separator("ESP");
					ImGui::Checkbox("Boxes", &g_Options.esp_player_boxes); ImGui::SameLine(); ImGuiEx::ColorEdit4("Enemies Visible   ", &g_Options.color_esp_enemy_visible);
					ImGui::Checkbox("Occluded ", &g_Options.esp_player_boxesOccluded); ImGui::SameLine(); ImGuiEx::ColorEdit4("Enemies Occluded      ", &g_Options.color_esp_enemy_occluded);

					ImGui::Checkbox("Names", &g_Options.esp_player_names);
					ImGui::Checkbox("Health", &g_Options.esp_player_health);
					ImGui::Checkbox("Weapon", &g_Options.esp_player_weapons);
					ImGui::Checkbox("Dropped Weapons", &g_Options.esp_dropped_weapons);
					ImGui::Spacing();
					ImGui::Separator("Chams");
					ImGui::Checkbox("Enabled ", &g_Options.chams_player_enabled); ImGui::SameLine(); ImGuiEx::ColorEdit4a("Enemy Visible ", &g_Options.color_chams_player_enemy_visible);
					ImGui::Checkbox("Visible shine##chams_enemies_visible_shine", &g_Options.player_enemies_shine);
					ImGui::SameLine();
					ImGuiEx::ColorEdit4("##color_chams_enemies_visible_shine", &g_Options.player_enemy_visible_shine);
					ImGui::Checkbox("Occluded  ", &g_Options.chams_player_ignorez); ImGui::SameLine(); ImGuiEx::ColorEdit4a("Enemy Occluded ", &g_Options.color_chams_player_enemy_occluded);
					ImGui::Combo("##Flat", &g_Options.chams_player_flat, "Normal\0Flat \0");
					break;
				}
				break;
			case 3:
				switch (subtab[3]) {
				case 0:
					ImGui::Separator("General");

					ImGui::Checkbox("Rank reveal", &g_Options.misc_showranks);
					ImGui::Checkbox("Spectator list", &g_Options.spectator_list);
					ImGui::Checkbox("Watermark##hc", &g_Options.misc_watermark);
					ImGui::Checkbox("Velocity", &g_Options.Velocity);
					ImGui::SameLine();
					ImGuiEx::ColorEdit4("##Velocity", &g_Options.Velocitycol);
					ImGui::Spacing();

					if (ImGui::BeginCombo("##Velocity", "Velocity", ImGuiComboFlags_NoArrowButton))
					{
						ImGui::Selectable("Outline", &g_Options.outline, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Last jump", &g_Options.lastjump, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Last jump outline", &g_Options.lastjumpoutline, ImGuiSelectableFlags_DontClosePopups);

						ImGui::EndCombo();
					}
					ImGui::Checkbox("Auto accept", &g_Options.autoaccept);
					ImGui::Checkbox("No flash", &g_Options.no_flash);
					ImGui::Checkbox("No smoke", &g_Options.no_smoke);
					ImGui::Checkbox("Sniper crosshair", &g_Options.sniper_xhair);
					break;
				}
				break;
			case 4:
				switch (subtab[4]) {
				case 0:
					ReadDirectory(g_Options.folder, cfgList);
					ImGui::Separator("Configs");
					if (!cfgList.empty())
					{
						ImGui::PushItemWidth(150.f);
						if (ImGui::BeginCombo("##SelectConfig", cfgList[selected].c_str(), ImGuiComboFlags_NoArrowButton))
						{
							for (size_t i = 0; i < cfgList.size(); i++)
							{
								if (ImGui::Selectable(cfgList[i].c_str(), i == selected))
									selected = i;
							}
							ImGui::EndCombo();

						}
						ImGui::PopItemWidth();

						ImGui::Separator("Actions");

						if (ImGui::Button(" Save Config"))
							g_Options.SaveSettings(cfgList[selected]);

						if (ImGui::Button(" Load Config"))
							g_Options.LoadSettings(cfgList[selected]);

						if (ImGui::Button(" Delete Config"))
						{
							g_Options.DeleteSettings(cfgList[selected]);
							selected = 0;
						}
					}
					break;
				case 1:

					break;
				}
				break;
			case 5:
				switch (subtab[5]) {
				case 0:

					break;
				}
				break;
			}
		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2{ (float)(10 + 120 + 10 + last_x / 2 + 10), 40 });
		ImGui::BeginChild("##FuncitonalTab2", ImVec2{ (float)(last_x / 2), y - 85 }, true);
		{
			switch (tab) {
			case 0:
				switch (subtab[0]) {
				case 0:

					break;
				case 1:
					ImGui::Separator("Settings");
					ImGui::Text(" Minimum Damage");
					ImGui::Spacing();
					ImGui::SliderInt("##damage", &ragebot->damage, 1, 130, "%i");
					
					ImGui::Text(" Hit Chance");
					ImGui::Spacing();
					ImGui::SliderInt("##hitchance", &ragebot->hitchance, 1, 100, "%i");

					if (ImGui::BeginCombo("##hitbox_filter", "Hitboxes", ImGuiComboFlags_NoArrowButton))
					{
						ImGui::Selectable("Head", &ragebot->hitboxes.head, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Upper Chest", &ragebot->hitboxes.upper_chest, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Chest", &ragebot->hitboxes.chest, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Lower Chest", &ragebot->hitboxes.lower_chest, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Hands", &ragebot->hitboxes.hands, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Legs", &ragebot->hitboxes.legs, ImGuiSelectableFlags_DontClosePopups);

						ImGui::EndCombo();
					}
					break;
				}
				break;
			case 1:
				switch (subtab[1]) {
				case 0:
					ImGui::Separator("Settings");
					ImGui::Text(" Fov");
					ImGui::Spacing();
					ImGui::SliderFloat("##Fov", &legitbot->fov, 0.f, 20.f, "%.f");
					if (legitbot->silent) {
						ImGui::Text(" Silent fov");
						ImGui::Spacing();
						ImGui::SliderFloat("##Silentfov", &legitbot->silent_fov, 0.f, 20.f, "%.f");
					}
					ImGui::Text(" Smooth");
					ImGui::Spacing();
					ImGui::SliderFloat("##Smooth", &legitbot->smooth, 1.f, 20.f, "%.f");

					if (!legitbot->silent) {
						ImGui::Text(" Shot delay");
						ImGui::Spacing();
						ImGui::SliderInt("##Shotdelay", &legitbot->shot_delay, 0, 1000, "%i");
					}
					ImGui::Text(" Kill delay");
					ImGui::Spacing();
					ImGui::SliderInt("##Killdelay", &legitbot->kill_delay, 0, 1000, "%i");

					ImGui::Spacing();

					ImGui::Checkbox("Enabled##rcs", &legitbot->rcs.enabled);

					//ImGui::SliderInt("##start", &settings->rcs.start, 1, 30, "Start: %i");
					ImGui::Text(" X");
					ImGui::Spacing();
					ImGui::SliderInt("##x", &legitbot->rcs.x, 0, 100, "%i");
					ImGui::Text(" Y");
					ImGui::Spacing();
					ImGui::SliderInt("##t", &legitbot->rcs.y, 0, 100, "%i");
					break;
				case 1:
					ImGui::Checkbox("Enabled autowall##autowall", &legitbot->autowall.enabled);
					ImGui::Spacing();
					ImGui::SameLine();
					ImGui::Text(" Min Damage");
					ImGui::Spacing();
					ImGui::SliderInt("##minDamage", &legitbot->autowall.min_damage, 1, 100, "%i");
					break;
				}
				break;
			case 2:
				switch (subtab[2]) {
				case 0:
					ImGui::Separator("Glow");
					ImGui::Checkbox("Enabled", &g_Options.glow_enabled);
					ImGui::SameLine();
					ImGuiEx::ColorEdit4a("##Enemy   ", &g_Options.color_glow_enemy);
					ImGui::Checkbox("Occluded   ", &g_Options.glow_enemiesOC);
					ImGui::SameLine();
					ImGuiEx::ColorEdit4a("##color_glow_enemiesOC   ", &g_Options.color_glow_enemyOC);
					const char* glow_enemies_type[] = {
						"Outline outer",
						"Pulse",
						"Outline inner"
					};
					if (ImGui::BeginCombo("##glow_enemies_type", glow_enemies_type[g_Options.glow_enemies_type], ImGuiComboFlags_NoArrowButton))
					{
						for (int i = 0; i < IM_ARRAYSIZE(glow_enemies_type); i++)
						{
							if (ImGui::Selectable(glow_enemies_type[i], i == g_Options.glow_enemies_type))
								g_Options.glow_enemies_type = i;
						}

						ImGui::EndCombo();
					}
					break;
				}
				break;
			case 3:
				switch (subtab[3]) {
				case 0:
					ImGui::Separator("Movement");

					ImGui::Checkbox("Bunny hop", &g_Options.misc_bhop);
					ImGui::Checkbox("Edge bug", &g_Options.edge_bug); ImGui::SameLine(); ImGui::Hotkey("  ", &g_Options.edge_bug_key);
					ImGui::Checkbox("Jump bug", &g_Options.jump_bug); ImGui::SameLine(); ImGui::Hotkey("   ", &g_Options.jump_bug_key);
					ImGui::Checkbox("Edge jump", &g_Options.edgejump.enabled); ImGui::SameLine(); ImGui::Hotkey("    ", &g_Options.edgejump.hotkey);
					ImGui::Checkbox("Duck in Air", &g_Options.edgejump.edge_jump_duck_in_air);
					break;
				}
				break;
			case 4:
				switch (subtab[4]) {
				case 0:
					ImGui::Separator("New config");
					ImGui::InputText("##configname", cfgName, 24);
					//ImGui::SameLine();
					if (ImGui::Button(" Create Config"))
					{
						if (strlen(cfgName))
							g_Options.SaveSettings(cfgName + std::string(".ini"));
					}
					break;
				case 1:

					break;
				}
				break;
			case 5:
				switch (subtab[5]) {
				case 0:

					break;
				}
				break;
			}
		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2{ 10 + 120 + 10, y - 85 + 40 + 10 });
		ImGui::BeginChild("##Info", ImVec2{ (float)(last_x + 10), 25 }, true);
		{
			ImGui::SetCursorPos(ImVec2(5, 4));
			ImGui::Text("Active user: ");
			ImGui::SameLine();
			ImGui::TextColored(ImColor(62, 123, 234), "Admin");
			ImGui::SameLine();
			ImGui::Text("  Cheat: ");
			ImGui::SameLine();
			ImGui::TextColored(ImColor(18, 211, 26), "Active");
		}
		ImGui::EndChild();
	}
	ImGui::End();
	ImGui::PopFont();
}

void Menu::Toggle()
{
    _visible = !_visible;
}

void Menu::CreateStyle()
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.900000f, 0.900000f, 0.900000f, 1.000000f));
	ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.600000f, 0.600000f, 0.600000f, 1.000000f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.000000f, 0.000000f, 0.000000f, 0.4f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.000000f, 0.000000f, 0.000000f, 0.000000f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.110000f, 0.110000f, 0.110000f, 0.920000f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.500000f, 0.500000f, 0.500000f, 0.500000f));
	ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.000000f, 0.000000f, 0.000000f, 0.000000f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.430000f, 0.430000f, 0.430000f, 0.390000f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.470000f, 0.470000f, 0.470000f, 0.400000f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.40000f, 0.40000f, 0.40000f, 0.690000f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.270000f, 0.270000f, 0.540000f, 0.830000f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.320000f, 0.320000f, 0.630000f, 0.870000f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.400000f, 0.400000f, 0.800000f, 0.200000f));
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.400000f, 0.400000f, 0.550000f, 0.800000f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.200000f, 0.250000f, 0.300000f, 0.00000f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.400000f, 0.400000f, 0.800000f, 0.00000f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.400000f, 0.400000f, 0.800000f, 0.00000f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.410000f, 0.390000f, 0.800000f, 0.00000f));
	ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.900000f, 0.900000f, 0.900000f, 0.500000f));
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.000000f, 1.000000f, 1.000000f, 0.300000f));
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.410000f, 0.390000f, 0.800000f, 0.600000f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.350000f, 0.400000f, 0.610000f, 0.0000f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.400000f, 0.480000f, 0.710000f, 0.0000f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.460000f, 0.540000f, 0.800000f, 0.000000f));
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.400000f, 0.400000f, 0.400000f, 0.450000f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.400000f, 0.400000f, 0.400000f, 0.800000f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.400000f, 0.400000f, 0.400000f, 0.800000f));
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.500000f, 0.500000f, 0.500000f, 0.600000f));
	ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImVec4(0.600000f, 0.600000f, 0.700000f, 1.000000f));
	ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImVec4(0.700000f, 0.700000f, 0.900000f, 1.000000f));
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, ImVec4(1.000000f, 1.000000f, 1.000000f, 0.160000f));
	ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, ImVec4(0.780000f, 0.820000f, 1.000000f, 0.600000f));
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImVec4(0.780000f, 0.820000f, 1.000000f, 0.900000f));
	ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(0.000000f, 0.000000f, 1.000000f, 0.350000f));
	ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(1.000000f, 1.000000f, 0.000000f, 0.900000f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.000000f,6.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.000000f,3.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8.000000f,4.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 4.000000f,4.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 21.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 1.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 1.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.000000f,0.500000f });
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.500000f,0.500000f });
	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.500000f,0.500000f });
}

