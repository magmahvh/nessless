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

	ImGui::RenderFrame(bb.Min, bb.Max, state ? ImColor(15, 15, 15) : ImColor(23, 23, 23), true, style.FrameRounding);
	window->DrawList->AddRect(bb.Min, bb.Max, ImColor(0, 0, 0));
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	if (state)
	{
		window->DrawList->AddLine(bb.Min, bb.Min + ImVec2(9, 0), ImColor(255, 255, 255), 1);
		window->DrawList->AddLine(bb.Min, bb.Min + ImVec2(0, 9), ImColor(255, 255, 255), 1);

		window->DrawList->AddLine(bb.Max - ImVec2(0, 1), bb.Max - ImVec2(10, 1), ImColor(255, 255, 255), 1);
		window->DrawList->AddLine(bb.Max - ImVec2(1, 1), bb.Max - ImVec2(1, 10), ImColor(255, 255, 255), 1);
	}

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
		ImGui::PushFont(g_SpectatorListFont);

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

	static int tab = 0;

	ImGui::SetNextWindowSize({ 560.000000f,360.000000f });

	ImGui::Begin("Edited", nullptr, flags);
	{
		ImVec2 p = ImGui::GetWindowPos();
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + 10, p.y + 10), ImVec2(p.x + 550, p.y + 350), ImColor(0.000000f, 0.000000f, 0.000000f, 0.4f), 0, 15); // main frame
		ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x + 0, p.y + 0), ImVec2(p.x + 560, p.y + 360), ImColor(0.000000f, 0.000000f, 0.000000f, 0.639216f), 0, 15, 1.000000);  // main frame
		ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x + 10, p.y + 10), ImVec2(p.x + 550, p.y + 350), ImColor(0.000000f, 0.000000f, 0.000000f, 0.639216f), 0, 15, 1.000000);  // main frame
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + 15, p.y + 60), ImVec2(p.x + 545, p.y + 345), ImColor(0.066667f, 0.066667f, 0.066667f, 1.000000f), 0, 15); // main frame
		ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x + 15, p.y + 60), ImVec2(p.x + 545, p.y + 345), ImColor(0.000001f, 0.000001f, 0.000001f, 1.000000f), 0, 15, 1.000000);  // main frame

		ImGui::SetCursorPos({ 15,17 });
		if (Tab("legitbot", { 125,35 }, tab == 0))
			tab = 0;

		ImGui::SetCursorPos({ 15 + 125 + 10,17 });
		if (Tab("visuals", { 125,35 }, tab == 1))
			tab = 1;

		ImGui::SetCursorPos({ 15 + 250 + 20,17 });
		if (Tab("misc", { 125,35 }, tab == 2))
			tab = 2;

		ImGui::SetCursorPos({ 15 + 375 + 30,17 });
		if (Tab("skins", { 125,35 }, tab == 3))
			tab = 3;

		if (tab == 0)
		{
			static int definition_index = WEAPON_INVALID;

			auto localPlayer = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
			if (g_EngineClient->IsInGame() && localPlayer && localPlayer->IsAlive() && localPlayer->m_hActiveWeapon() && localPlayer->m_hActiveWeapon()->IsGun())
				definition_index = localPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex();
			else
				definition_index = WEAPON_INVALID;
			if (definition_index != WEAPON_INVALID) 
			{
				ImGui::SetCursorPos({ 21,65 });
				ImGui::BeginChild("##1", { 166,276 });
				{
					ImGui::Separator("General");
					auto settings = &g_Options.weapons[definition_index].legit;

					ImGui::Checkbox("Enabled", &settings->enabled);
					//ImGui::Checkbox("Friendly fire", &settings->deathmatch);
					ImGui::Combo("Silent", &settings->silent2, "Off\0Silent \0Perfect silent\0");
					ImGui::Checkbox("Flash check", &settings->flash_check);
					ImGui::Checkbox("Smoke check", &settings->smoke_check);
					ImGui::Checkbox("Auto-pistol", &settings->autopistol);

					if (ImGui::BeginCombo("##hitbox_filter", "Hitboxes", ImGuiComboFlags_NoArrowButton))
					{
						ImGui::Selectable("Head", &settings->hitboxes.head, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Chest", &settings->hitboxes.chest, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Hands", &settings->hitboxes.hands, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Legs", &settings->hitboxes.legs, ImGuiSelectableFlags_DontClosePopups);

						ImGui::EndCombo();
					}


				}
				ImGui::EndChild();

				ImGui::SetCursorPos({ 31 + 166,65 });
				ImGui::BeginChild("##2", { 166,276 });
				{
					auto settings = &g_Options.weapons[definition_index].legit;
					ImGui::Separator("Misc");

					ImGui::Text("  Fov");
					ImGui::Spacing();
					ImGui::SliderFloat("##Fov", &settings->fov, 0.f, 20.f, "%.f");
					ImGui::Spacing();
					if (settings->silent2) {
						ImGui::Text("  Silent fov");
						ImGui::Spacing();
						ImGui::SliderFloat("##Silentfov", &settings->silent_fov, 0.f, 20.f, "%.f");
					}
					ImGui::Text("  Smooth");
					ImGui::Spacing();
					ImGui::SliderFloat("##Smooth", &settings->smooth, 1.f, 20.f, "%.f");

					ImGui::Separator("Delays");


					if (!settings->silent2) {
						ImGui::Text("  Shot delay");
						ImGui::Spacing();
						ImGui::SliderInt("##Shotdelay", &settings->shot_delay, 0, 1000, "%i");
					}
					ImGui::Text("  Kill delay");
					ImGui::Spacing();
					ImGui::SliderInt("##Killdelay", &settings->kill_delay, 0, 1000, "%i");
					ImGui::Separator("RCS");

					ImGui::Checkbox("Enabled##rcs", &settings->rcs.enabled);

					const char* rcs_types[] = {
						"Type: Standalone",
						"Type: Aim"
					};


					if (ImGui::BeginCombo("##type", rcs_types[settings->rcs.type], ImGuiComboFlags_NoArrowButton))
					{
						for (int i = 0; i < IM_ARRAYSIZE(rcs_types); i++)
						{
							if (ImGui::Selectable(rcs_types[i], i == settings->rcs.type))
								settings->rcs.type = i;
						}

						ImGui::EndCombo();
					}
					//ImGui::SliderInt("##start", &settings->rcs.start, 1, 30, "Start: %i");
					ImGui::SliderInt("  X", &settings->rcs.x, 0, 100, "%i");
					ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();
					ImGui::SliderInt("  Y", &settings->rcs.y, 0, 100, "%i");
				}
				ImGui::EndChild();

				ImGui::SetCursorPos({ 41 + 166 + 166,65 });
				ImGui::BeginChild("##3", { 166,276 });
				{
					float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

					auto settings = &g_Options.weapons[definition_index].legit;



					ImGui::Separator("AutoWall ");

					ImGui::Checkbox("Enabled##autowall", &settings->autowall.enabled);
					ImGui::Spacing();
					ImGui::SameLine();
					ImGui::Text("Min Damage");
					ImGui::Spacing();
					ImGui::SliderInt("##minDamage", &settings->autowall.min_damage, 1, 100, "%i");



					ImGui::Separator("Auto-fire");
					ImGui::Checkbox("Enabled##autofire", &settings->autofire.enabled);
					ImGui::SameLine(group_w - 50);
					ImGui::Hotkey("##autofire", &settings->autofire.hotkey);
				}
				ImGui::EndChild();
			}
		}
		else if (tab == 1)
		{
			ImGui::SetCursorPos({ 21,65 });
			ImGui::BeginChild("##1", { 166,276 });
			{
				ImGui::Separator("ESP");
				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

			//	ImGui::Checkbox("Enabled",&g_Options.esp_enabled);
				//ImGui::Checkbox("Team check",&g_Options.esp_enemies_only);
				ImGui::Checkbox("Boxes",&g_Options.esp_player_boxes); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4("Enemies Visible   ",&g_Options.color_esp_enemy_visible);
				ImGui::Checkbox("Occluded ",&g_Options.esp_player_boxesOccluded); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4("Enemies Occluded      ",&g_Options.color_esp_enemy_occluded);

				ImGui::Checkbox("Names",&g_Options.esp_player_names);
				ImGui::Checkbox("Health",&g_Options.esp_player_health);
				//ImGui::Checkbox("Armour",&g_Options.esp_player_armour);
				ImGui::Checkbox("Weapon",&g_Options.esp_player_weapons);
				//ImGui::Checkbox("Snaplines",&g_Options.esp_player_snaplines);
				ImGui::Checkbox("Dropped Weapons",&g_Options.esp_dropped_weapons);
				//ImGuiEx::ColorEdit4("Allies Visible",&g_Options.color_esp_ally_visible);
				//ImGuiEx::ColorEdit4("Enemies Visible",&g_Options.color_esp_enemy_visible);
				//ImGuiEx::ColorEdit4("Allies Occluded",&g_Options.color_esp_ally_occluded);
				//ImGuiEx::ColorEdit4("Enemies Occluded",&g_Options.color_esp_enemy_occluded);
				//ImGuiEx::ColorEdit4("Dropped Weapons",&g_Options.color_esp_weapons);
			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 31 + 166,65 });
			ImGui::BeginChild("##2", { 166,276 });
			{
				ImGui::Separator("Chams");

				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				ImGui::Checkbox("Enabled ",&g_Options.chams_player_enabled); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4a("Enemy Visible ",&g_Options.color_chams_player_enemy_visible);
				ImGui::Checkbox("Visible shine##chams_enemies_visible_shine", &g_Options.player_enemies_shine);
				ImGui::SameLine(group_w - 20);
				ImGuiEx::ColorEdit4("##color_chams_enemies_visible_shine", &g_Options.player_enemy_visible_shine);
				//ImGui::Checkbox("Team Check",&g_Options.chams_player_enemies_only);
				//ImGui::Checkbox("Wireframe",&g_Options.chams_player_wireframe);

				ImGui::Checkbox("Occluded  ",&g_Options.chams_player_ignorez); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4a("Enemy Occluded ",&g_Options.color_chams_player_enemy_occluded);

				//ImGui::Checkbox("Flat",&g_Options.chams_player_flat);
				ImGui::Combo("##Flat",&g_Options.chams_player_flat, "Normal\0Flat \0");

				//ImGui::Checkbox("Glass",&g_Options.chams_player_glass);
				//ImGuiEx::ColorEdit4("Ally (Visible)",&g_Options.color_chams_player_ally_visible);
				//ImGuiEx::ColorEdit4("Ally (Occluded)",&g_Options.color_chams_player_ally_occluded);

			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 41 + 166 + 166,65 });
			ImGui::BeginChild("##3", { 166,276 });
			{
				ImGui::Separator("Glow");

				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				ImGui::Checkbox("Enabled",&g_Options.glow_enabled);
				ImGui::SameLine(group_w - 20);
				ImGuiEx::ColorEdit4a("##Enemy   ",&g_Options.color_glow_enemy);
				ImGui::Checkbox("Occluded   ", &g_Options.glow_enemiesOC);
				ImGui::SameLine(group_w - 20);
				ImGuiEx::ColorEdit4a("##color_glow_enemiesOC   ",&g_Options.color_glow_enemyOC);
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
			}
			ImGui::EndChild();
		}
		else if (tab == 2)
		{
			ImGui::SetCursorPos({ 21,65 });
			ImGui::BeginChild("##1", { 166,276 });
			{				
				ImGui::Separator("General");

				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				ImGui::Checkbox("Rank reveal",&g_Options.misc_showranks);
				ImGui::Checkbox("Spectator list", &g_Options.spectator_list);
				ImGui::Checkbox("Watermark##hc",&g_Options.misc_watermark);
				ImGui::Checkbox("Velocity", &g_Options.Velocity);
				ImGui::SameLine(group_w - 20);
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

			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 31 + 166,65 });
			ImGui::BeginChild("##2", { 166,276 });
			{
				ImGui::Separator("Movement");

				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				ImGui::Checkbox("Bunny hop", &g_Options.misc_bhop);
				ImGui::Checkbox("Edge bug", &g_Options.edge_bug); ImGui::SameLine(group_w - 50);          ImGui::Hotkey("  ", &g_Options.edge_bug_key);
				ImGui::Checkbox("Jump bug", &g_Options.jump_bug); ImGui::SameLine(group_w - 50);          ImGui::Hotkey("   ", &g_Options.jump_bug_key);
				ImGui::Checkbox("Edge jump", &g_Options.edgejump.enabled); ImGui::SameLine(group_w - 50); ImGui::Hotkey("    ", &g_Options.edgejump.hotkey);
				ImGui::Checkbox("Duck in Air", &g_Options.edgejump.edge_jump_duck_in_air);
			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 41 + 166 + 166,65 });
			ImGui::BeginChild("##3", { 166,276 });
			{
				ImGui::Separator("Config");

				static int selected = 0;
				static char cfgName[64];

				std::vector<std::string> cfgList;
				ReadDirectory(g_Options.folder, cfgList);
				ImGui::PushItemWidth(150.f);
				if (!cfgList.empty())
				{
					if (ImGui::BeginCombo("##SelectConfig", cfgList[selected].c_str(), ImGuiComboFlags_NoArrowButton))
					{
						for (size_t i = 0; i < cfgList.size(); i++)
						{
							if (ImGui::Selectable(cfgList[i].c_str(), i == selected))
								selected = i;
						}
						ImGui::EndCombo();

					}
					if (ImGui::Button(" Save   Config"))
						g_Options.SaveSettings(cfgList[selected]);
					//ImGui::SameLine();
					if (ImGui::Button(" Load   Config"))
						g_Options.LoadSettings(cfgList[selected]);
					//ImGui::SameLine();
					if (ImGui::Button(" Delete Config"))
					{
						g_Options.DeleteSettings(cfgList[selected]);
						selected = 0;
					}
					//	ImGui::Separator();
				}
				ImGui::Spacing();
				ImGui::SameLine();
				ImGui::InputText("##configname", cfgName, 24);
				//ImGui::SameLine();
				if (ImGui::Button(" Create Config"))
				{
					if (strlen(cfgName))
						g_Options.SaveSettings(cfgName + std::string(".ini"));
				}
				ImGui::PopItemWidth();
			}
			ImGui::EndChild();
		}
		else if (tab == 3)
	{
	static std::string selected_weapon_name = "";
	static std::string selected_skin_name = "";
	static auto definition_vector_index = 0;
	auto& entries = g_Options.changers.skin.m_items;
	ImGui::SetCursorPos({ 21,65 });
	ImGui::BeginChild("##1", { 166,276 });
	{
	/*	ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SameLine();*/
		//	ImGui::ListBoxHeader("##sjinstab",ImVec2(155,245));
		ImGui::Spacing();
		ImGui::Spacing();

		{
			for (size_t w = 0; w < k_weapon_names.size(); w++)
			{
				switch (w)
				{
				case 0:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "knife");
					break;
				case 2:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "glove");
					break;
				case 4:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "pistols");
					break;
				case 14:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "semi-rifle");
					break;
				case 21:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "rifle");
					break;
				case 28:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "sniper-rifle");
					break;
				case 32:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "machingun");
					break;
				case 34:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "shotgun");
					break;
				}

				if (ImGui::Selectable(k_weapon_names[w].name, definition_vector_index == w))
				{
					definition_vector_index = w;
				}
			}
		}
		//ImGui::ListBoxFooter();

	}
	ImGui::EndChild();

	ImGui::SetCursorPos({ 31 + 166,65 });
	ImGui::BeginChild("##2", { 166,276 });
	{			
		ImGui::Spacing();
		ImGui::Spacing();
		auto& selected_entry = entries[k_weapon_names[definition_vector_index].definition_index];
		auto& satatt = g_Options.changers.skin.statrack_items[k_weapon_names[definition_vector_index].definition_index];
		selected_entry.definition_index = k_weapon_names[definition_vector_index].definition_index;
		selected_entry.definition_vector_index = definition_vector_index;
		if (selected_entry.definition_index == WEAPON_KNIFE || selected_entry.definition_index == WEAPON_KNIFE_T)
		{
			ImGui::PushItemWidth(160.f);

			ImGui::Combo("", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_knife_names.at(idx).name;
					return true;
				}, nullptr, k_knife_names.size(), 10);
			selected_entry.definition_override_index = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;

		}
		else if (selected_entry.definition_index == GLOVE_T_SIDE || selected_entry.definition_index == GLOVE_CT_SIDE)
		{
			ImGui::PushItemWidth(160.f);

			ImGui::Combo("", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_glove_names.at(idx).name;
					return true;
				}, nullptr, k_glove_names.size(), 10);
			selected_entry.definition_override_index = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
		}
		else {
			static auto unused_value = 0;
			selected_entry.definition_override_vector_index = 0;
		}

		if (selected_entry.definition_index != GLOVE_T_SIDE &&
			selected_entry.definition_index != GLOVE_CT_SIDE &&
			selected_entry.definition_index != WEAPON_KNIFE &&
			selected_entry.definition_index != WEAPON_KNIFE_T)
		{
			selected_weapon_name = k_weapon_names_preview[definition_vector_index].name;
		}
		else
		{
			if (selected_entry.definition_index == GLOVE_T_SIDE ||
				selected_entry.definition_index == GLOVE_CT_SIDE)
			{
				selected_weapon_name = k_glove_names_preview.at(selected_entry.definition_override_vector_index).name;
			}
			if (selected_entry.definition_index == WEAPON_KNIFE ||
				selected_entry.definition_index == WEAPON_KNIFE_T)
			{
				selected_weapon_name = k_knife_names_preview.at(selected_entry.definition_override_vector_index).name;
			}
		}
		if (skins_parsed)
		{
			static char filter_name[32];
			std::string filter = filter_name;

			bool is_glove = selected_entry.definition_index == GLOVE_T_SIDE ||
				selected_entry.definition_index == GLOVE_CT_SIDE;

			bool is_knife = selected_entry.definition_index == WEAPON_KNIFE ||
				selected_entry.definition_index == WEAPON_KNIFE_T;

			int cur_weapidx = 0;
			if (!is_glove && !is_knife)
			{
				cur_weapidx = k_weapon_names[definition_vector_index].definition_index;
				//selected_weapon_name = k_weapon_names_preview[definition_vector_index].name;
			}
			else
			{
				if (selected_entry.definition_index == GLOVE_T_SIDE ||
					selected_entry.definition_index == GLOVE_CT_SIDE)
				{
					cur_weapidx = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
				}
				if (selected_entry.definition_index == WEAPON_KNIFE ||
					selected_entry.definition_index == WEAPON_KNIFE_T)
				{
					cur_weapidx = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;

				}
			}

			/*	ImGui::InputText("name filter [?]", filter_name, sizeof(filter_name));
				if (ImGui::ItemsToolTipBegin("##skinfilter"))
				{
					ImGui::Checkbox("show skins for selected weapon", &g_Options.changers.skin.show_cur);
					ImGui::ItemsToolTipEnd();
				}*/

			auto weaponName = weaponnames(cur_weapidx);
			/*ImGui::Spacing();

			ImGui::Spacing();
			ImGui::SameLine();*/
			//ImGui::ListBoxHeader("##sdsdadsdadas", ImVec2(155, 245));
			{
				if (selected_entry.definition_index != GLOVE_T_SIDE && selected_entry.definition_index != GLOVE_CT_SIDE)
				{
					if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
					{
						selected_entry.paint_kit_vector_index = -1;
						selected_entry.paint_kit_index = -1;
						selected_skin_name = "";
					}

					int lastID = ImGui::GetItemID();
					for (size_t w = 0; w < k_skins.size(); w++)
					{
						for (auto names : k_skins[w].weaponName)
						{
							std::string name = k_skins[w].name;

							if (g_Options.changers.skin.show_cur)
							{
								if (names != weaponName)
									continue;
							}

							if (name.find(filter) != name.npos)
							{
								ImGui::PushID(lastID++);

								ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(is_knife && g_Options.changers.skin.show_cur ? 6 : k_skins[w].rarity));
								{
									if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
									{
										selected_entry.paint_kit_vector_index = w;
										selected_entry.paint_kit_index = k_skins[selected_entry.paint_kit_vector_index].id;
										selected_skin_name = k_skins[w].name_short;
									}
								}
								ImGui::PopStyleColor();

								ImGui::PopID();
							}
						}
					}
				}
				else
				{
					int lastID = ImGui::GetItemID();

					if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
					{
						selected_entry.paint_kit_vector_index = -1;
						selected_entry.paint_kit_index = -1;
						selected_skin_name = "";
					}

					for (size_t w = 0; w < k_gloves.size(); w++)
					{
						for (auto names : k_gloves[w].weaponName)
						{
							std::string name = k_gloves[w].name;
							//name += " | ";
							//name += names;

							if (g_Options.changers.skin.show_cur)
							{
								if (names != weaponName)
									continue;
							}

							if (name.find(filter) != name.npos)
							{
								ImGui::PushID(lastID++);

								ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(6));
								{
									if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
									{
										selected_entry.paint_kit_vector_index = w;
										selected_entry.paint_kit_index = k_gloves[selected_entry.paint_kit_vector_index].id;
										selected_skin_name = k_gloves[selected_entry.paint_kit_vector_index].name_short;
									}
								}
								ImGui::PopStyleColor();

								ImGui::PopID();
							}
						}
					}
				}
			}
			//	ImGui::ListBoxFooter();
		}
		else
		{
			ImGui::Text("skins parsing, wait...");
		}
		//ImGui::Checkbox("skin preview", &g_Options.changers.skin.skin_preview);
		/*ImGui::Checkbox("stattrak##2", &selected_entry.stat_trak);
		ImGui::InputInt("seed", &selected_entry.seed);
		ImGui::InputInt("stattrak", &satatt.statrack_new.counter);
		ImGui::SliderFloat("wear", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5);
		*/


	}
	ImGui::EndChild();

	ImGui::SetCursorPos({ 41 + 166 + 166,65 });
	ImGui::BeginChild("##3", { 166,276 });
	{			
		if (ImGui::Button(" update skin"))
		{
			//	if (next_enb_time <= g_GlobalVars->curtime)
			{
				static auto clear_hud_weapon_icon_fn =
					reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(
						Utils::PatternScan2("client.dll", "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C 89 5D FC"));

				auto element = FindHudElement<std::uintptr_t*>("CCSGO_HudWeaponSelection");

				if (element)
				{
					auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0xa0);
					if (hud_weapons != nullptr)
					{

						if (*hud_weapons->get_weapon_count())
						{
							for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
								i = clear_hud_weapon_icon_fn(hud_weapons, i);

							typedef void(*ForceUpdate) (void);
							static ForceUpdate FullUpdate = (ForceUpdate)Utils::PatternScan2("engine.dll", "A1 ? ? ? ? B9 ? ? ? ? 56 FF 50 14 8B 34 85");
							FullUpdate();

							g_ClientState->ForceFullUpdate();
						}
					}
				}

				//next_enb_time = g_GlobalVars->curtime + 1.f;
			}
		}

	}
	ImGui::EndChild();
	}
	}
	ImGui::End();
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

