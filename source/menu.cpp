#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>
#include <filesystem>
#include <string>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"
#include "config.hpp"
#include "../valve_sdk/interfaces/CClientState.hpp"

#include "helpers/bind.h"


#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"


// =========================================================
// 
// These are the tabs on the sidebar
// 
// =========================================================

std::vector <std::string> files;
const char* config_edit = "";
const char* config_active = "";

constexpr static float get_sidebar_item_width() { return 150.0f; }
constexpr static float get_sidebar_item_height() { return  50.0f; }

namespace ImGui
{

	bool Tab(const char* label, const char* desc, const ImVec2& size_arg, const bool selected)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		static float sizeplus = 0.f;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

		ImVec2 pos = window->DC.CursorPos;

		ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ImGui::ItemSize(size, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

		if (hovered) {
			window->DrawList->AddRectFilled({ bb.Min.x + 1,bb.Min.y }, { bb.Max.x - 1, bb.Max.y }, ImColor(39, 39, 39, 255)); 
			window->DrawList->AddRectFilled({ bb.Min.x,bb.Min.y + 1 }, { bb.Max.x, bb.Max.y - 1 }, ImColor(39, 39, 39, 255)); 
		}
		else {
			window->DrawList->AddRectFilled({ bb.Min.x + 1,bb.Min.y }, { bb.Max.x - 1, bb.Max.y }, ImColor(43, 43, 43, 255)); 
			window->DrawList->AddRectFilled({ bb.Min.x,bb.Min.y + 1 }, { bb.Max.x, bb.Max.y - 1 }, ImColor(43, 43, 43, 255)); 
		}

		window->DrawList->AddText(ImVec2(bb.Min.x + 50, bb.Min.y + 7), ImColor(255, 255, 255, 255), label);
		window->DrawList->AddText(ImVec2(bb.Min.x + 50, bb.Min.y + 30), ImColor(200, 200, 200, 255), desc);

		return pressed;
	}

	bool SubTab(const char* label, const ImVec2& size_arg, const bool selected)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		static float sizeplus = 0.f;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

		ImVec2 pos = window->DC.CursorPos;

		ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ImGui::ItemSize(size, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);
		
		if (selected) {
			window->DrawList->AddRectFilledMultiColor({ bb.Min.x + 1,bb.Min.y }, { bb.Max.x - 1, bb.Max.y }, ImColor(150, 170, 235, 90), ImColor(150, 170, 235, 0), ImColor(150, 170, 235, 0), ImColor(150, 170, 235, 90));
			window->DrawList->AddRectFilledMultiColor({ bb.Min.x,bb.Min.y + 1 }, { bb.Max.x, bb.Max.y - 1 }, ImColor(150, 170, 235, 90), ImColor(150, 170, 235, 0), ImColor(150, 170, 235, 0), ImColor(150, 170, 235, 90));
		}
		else {
			window->DrawList->AddRectFilled({ bb.Min.x + 1,bb.Min.y }, { bb.Max.x - 1, bb.Max.y }, ImColor(43, 43, 43, 255));
			window->DrawList->AddRectFilled({ bb.Min.x,bb.Min.y + 1 }, { bb.Max.x, bb.Max.y - 1 }, ImColor(43, 43, 43, 255));
		}

		if (hovered) {
			window->DrawList->AddRectFilled({ bb.Min.x + 1,bb.Min.y }, { bb.Max.x - 1, bb.Max.y }, ImColor(58, 58, 58, 80));
			window->DrawList->AddRectFilled({ bb.Min.x,bb.Min.y + 1 }, { bb.Max.x, bb.Max.y - 1 }, ImColor(58, 58, 58, 80));
		}

		window->DrawList->AddText(ImVec2(bb.Min.x + 8, bb.Min.y + size_arg.y / 2 - ImGui::CalcTextSize(label).y / 2), selected ? ImColor(160, 180, 245, 255) : ImColor(255, 255, 255, 255), label);

		return pressed;
	}

	bool arrow_back(const char* label, const ImVec2& size_arg, const bool selected)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		static float sizeplus = 0.f;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

		ImVec2 pos = window->DC.CursorPos;

		ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ImGui::ItemSize(size, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

		window->DrawList->AddText(ImVec2(bb.Min.x + size_arg.x / 2 - ImGui::CalcTextSize(label).x / 2, bb.Min.y + size_arg.y / 2 - ImGui::CalcTextSize(label).y / 2), selected ? ImColor(255, 255, 255, 255) : ImColor(140, 140, 140, 255), label);

		return pressed;
	}

	bool SelectableBox(const char* label, bool* selected)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		static float sizeplus = 0.f;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

		ImVec2 pos = window->DC.CursorPos;

		ImVec2 size = ImGui::CalcItemSize(ImVec2{0, 0}, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ImGui::ItemSize(size, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

		if (pressed)
			*selected = !*selected;

		window->DrawList->AddRectFilled({ bb.Min.x, bb.Min.y }, { bb.Max.x, bb.Max.y }, ImColor(60, 60, 60, 80));
		window->DrawList->AddText(ImVec2(bb.Min.x + size.x / 2 - ImGui::CalcTextSize(label).x / 2, bb.Min.y + size.y / 2 - ImGui::CalcTextSize(label).y / 2), *selected ? ImColor(255, 255, 255, 255) : ImColor(140, 140, 140, 255), label);

		return pressed;
	}

	bool Popup(const char* label, const ImVec2& size_arg )
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		static float sizeplus = 0.f;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

		ImVec2 pos = window->DC.CursorPos;

		ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ImGui::ItemSize(size, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

		if (ImGui::IsItemHovered() && ImGui::GetIO().MouseClicked[1])
			ImGui::OpenPopup(label);

		return pressed;
	}


    inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
    {
        auto clr = ImVec4{
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };

        if(ImGui::ColorEdit4(label, &clr.x, show_alpha)) {
            v->SetColor(clr.x, clr.y, clr.z, clr.w);
            return true;
        }
        return false;
    }
    inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

void cfglist_update() {
	Config::Get().Files();
	files = Config::Get().items;
}

__forceinline void vert_line(float y, ImColor col) {
	ImGuiStyle* Style = &ImGui::GetStyle();
	Style->Colors[ImGuiCol_ChildBg] = ImColor(46, 46, 46);
	ImGui::BeginChild("##line_vert", ImVec2{ 2, y }, false); { } ImGui::EndChild();
	Style->Colors[ImGuiCol_ChildBg] = col;
}

__forceinline void gor_line(float x, ImColor col) {
	ImGuiStyle* Style = &ImGui::GetStyle();
	Style->Colors[ImGuiCol_ChildBg] = ImColor(46, 46, 46);
	ImGui::BeginChild("##line_gor", ImVec2{ x, 2 }, false); { } ImGui::EndChild();
	Style->Colors[ImGuiCol_ChildBg] = col;
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

void Menu::Render()
{
	ImGui::GetIO().MouseDrawCursor = _visible;

    if(!_visible)
        return;
    
	ImVec2 pos_esp_preview;

	ImGuiStyle* Style = &ImGui::GetStyle();
	Style->Colors[ImGuiCol_Text] = ImColor(255, 255, 255);
	Style->Colors[ImGuiCol_WindowBg] = ImColor(32, 32, 32);
	Style->Colors[ImGuiCol_ChildBg] = ImColor(39, 39, 39);
	Style->WindowBorderSize = 0;
	Style->WindowRounding = 0;
	Style->ChildRounding = 8;
    static int active_sidebar_tab = 0;
	static float x = 520, y = 570;

	static char cfg_name[64] = { 0 };
	static char cfg_name_new[64] = { 0 };
	const int tabs_count = 5;
	static char* tabs[tabs_count][2] = {
		{ "Ragebot", "Install additional settings for the ragebot" },
		{ "Anti-Aim", "Set up your antiaims so that they miss!" },
		{ "Visual", "Visual functionality, warning grenades and much more" },
		{ "Misc", "Configure additional functionality" },
		{ "Configs", "Save your settings" },
	};

	static char* weapon_names[] = {
		"SCAR20", "AWP", "SCOUT", "HEAVY", "RIFLES", "PISTOLS", "GENERAL",
	};

	static int active_tab = 0;
	static int active_subtab_rage = 0;
	static int active_subtab_aa = 0;
	static int active_subtab_esp = 0;
	static int active_subtab_misc = 0;
	static int active_subtab_cfg = 0;

	static int active_weapon_edit = 0;
	static int active_visuals_edit = 0;
	std::string active_visuals_team = "enemy";

	auto name = "CHEAT";

    //ImGui::PushStyle(_style);

    ImGui::SetNextWindowPos(ImVec2{ 120, 120 }, ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2{ x, y }, ImGuiSetCond_Once);
	// quick fix

	if (ImGui::Begin(name,
		&_visible,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar)) {

		ImGui::SetCursorPos(ImVec2{ 0, 0 });
		ImGui::BeginChild("##Header", ImVec2{ x, 40 }, false);
		{
			ImGui::SetCursorPos(ImVec2{ 0, 0 });
			if (ImGui::arrow_back("<", ImVec2(40, 40), active_tab > 0 ? true : false))
				active_tab = 0;

			ImGui::SetCursorPos(ImVec2{ 60, 12 });
			ImGui::Text(active_tab == 0 ? name : tabs[active_tab - 1][0]);

			ImGui::SetCursorPos(ImVec2{ 40, 0 });
			vert_line(40, ImColor(39, 39, 39));

			ImGui::SetCursorPos(ImVec2{ 0, 38 });
			gor_line(x, ImColor(39, 39, 39)); 
		}
		ImGui::EndChild();

		if (active_tab == 0) {
			for (int i = 0; i < tabs_count; ++i) {
				ImGui::SetCursorPos(ImVec2(10, 50 + 60 * i));
				if (ImGui::Tab(tabs[i][0], tabs[i][1], ImVec2(x - 20, 50), active_tab == (i + 1) ? true : false))
					active_tab = (i + 1);
			}
		}
		else {
			ImGui::SetCursorPos(ImVec2{ 10, 50 });
			ImGui::BeginChild("##subtabs", ImVec2{ 150, y - 60 }, false);
			{
				switch (active_tab) {
				case 1: /*rage*/ {
					ImGui::SetCursorPos(ImVec2(5, 25 * 0.3));
					ImGui::Text("AIMBOT");

					ImGui::SetCursorPos(ImVec2(5, 25 * 1));
					if (ImGui::SubTab("General", ImVec2(140, 25), active_subtab_rage == 0 ? true : false))
						active_subtab_rage = 0;

					ImGui::SetCursorPos(ImVec2(5, 25 * 2));
					if (ImGui::SubTab("Exploits", ImVec2(140, 25), active_subtab_rage == 1 ? true : false))
						active_subtab_rage = 1;

					ImGui::SetCursorPos(ImVec2(5, 25 * 3));
					if (ImGui::SubTab("Fakelags", ImVec2(140, 25), active_subtab_rage == 2 ? true : false))
						active_subtab_rage = 2;

					ImGui::SetCursorPos(ImVec2(5, 25 * 4.3));
					ImGui::Text("WEAPON");
					for (int weapon_idx = 0; weapon_idx < 7; ++weapon_idx) {
						ImGui::SetCursorPos(ImVec2(5, 125 + 25 * weapon_idx));
						if (ImGui::SubTab(weapon_names[weapon_idx], ImVec2(140, 25), active_subtab_rage == 3 + weapon_idx ? true : false)) {
							active_subtab_rage = 3 + weapon_idx;
							active_weapon_edit = weapon_idx;
						}
					}

					break;
				}
				case 2: /*aa*/ {
					ImGui::SetCursorPos(ImVec2(5, 25 * 0.3));
					ImGui::Text("GENERAL");

					ImGui::SetCursorPos(ImVec2(5, 25 * 1));
					if (ImGui::SubTab("General", ImVec2(140, 25), active_subtab_aa == 0 ? true : false))
						active_subtab_aa = 0;

					ImGui::SetCursorPos(ImVec2(5, 25 * 2));
					if (ImGui::SubTab("Desync", ImVec2(140, 25), active_subtab_aa == 1 ? true : false))
						active_subtab_aa = 1;

					ImGui::SetCursorPos(ImVec2(5, 25 * 3));
					if (ImGui::SubTab("Other", ImVec2(140, 25), active_subtab_aa == 2 ? true : false))
						active_subtab_aa = 2;

					break;
				}
				case 3: /*visual*/ {
					ImGui::SetCursorPos(ImVec2(5, 25 * 0.3));
					ImGui::Text("ENEMY");

					ImGui::SetCursorPos(ImVec2(5, 25 * 1));
					if (ImGui::SubTab("ESP", ImVec2(140, 25), active_subtab_esp == 0 ? true : false)) {
						active_subtab_esp = 0;
					}

					ImGui::SetCursorPos(ImVec2(5, 25 * 2));
					if (ImGui::SubTab("Chams", ImVec2(140, 25), active_subtab_esp == 1 ? true : false)) {
						active_subtab_esp = 1;
					}

					ImGui::SetCursorPos(ImVec2(5, 25 * 3.3));
					ImGui::Text("LOCAL");

					/*ImGui::SetCursorPos(ImVec2(5, 25 * 4));
					if (ImGui::SubTab("ESP local", ImVec2(140, 25), active_subtab_esp == 2 ? true : false)) {
						active_subtab_esp = 2;
						active_visuals_edit = 1;
					}*/

					ImGui::SetCursorPos(ImVec2(5, 25 * 4));
					if (ImGui::SubTab("Chams local", ImVec2(140, 25), active_subtab_esp == 2 ? true : false)) {
						active_subtab_esp = 2;
					}

					ImGui::SetCursorPos(ImVec2(5, 25 * 5));
					if (ImGui::SubTab("Indicators", ImVec2(140, 25), active_subtab_esp == 3 ? true : false))
						active_subtab_esp = 3;

					ImGui::SetCursorPos(ImVec2(5, 25 * 6));
					if (ImGui::SubTab("View", ImVec2(140, 25), active_subtab_esp == 4 ? true : false))
						active_subtab_esp = 4;

					ImGui::SetCursorPos(ImVec2(5, 25 * 7.3));
					ImGui::Text("WORLD");

					ImGui::SetCursorPos(ImVec2(5, 25 * 8));
					if (ImGui::SubTab("Main", ImVec2(140, 25), active_subtab_esp == 5 ? true : false))
						active_subtab_esp = 5;

					ImGui::SetCursorPos(ImVec2(5, 25 * 9));
					if (ImGui::SubTab("Other", ImVec2(140, 25), active_subtab_esp == 6 ? true : false))
						active_subtab_esp = 6;

					ImGui::SetCursorPos(ImVec2(5, 25 * 10));
					if (ImGui::SubTab("Impacts", ImVec2(140, 25), active_subtab_esp == 7 ? true : false))
						active_subtab_esp = 7;

					break;
				}
				case 4: /*misc*/ {
					ImGui::SetCursorPos(ImVec2(5, 25 * 0.3));
					ImGui::Text("GENERAL");

					ImGui::SetCursorPos(ImVec2(5, 25 * 1));
					if (ImGui::SubTab("General misc", ImVec2(140, 25), active_subtab_misc == 0 ? true : false))
						active_subtab_misc = 0;

					ImGui::SetCursorPos(ImVec2(5, 25 * 2));
					if (ImGui::SubTab("Buy-bot", ImVec2(140, 25), active_subtab_misc == 1 ? true : false))
						active_subtab_misc = 1;

					ImGui::SetCursorPos(ImVec2(5, 25 * 3));
					if (ImGui::SubTab("Other", ImVec2(140, 25), active_subtab_misc == 2 ? true : false))
						active_subtab_misc = 2;

					break;
				}
				case 5: /*configs*/ {
					ImGui::SetCursorPos(ImVec2(5, 25 * 0.3));
					ImGui::Text("CONFIGS");
					ImGui::SetCursorPos(ImVec2(5, 25 * 1));
					ImGui::InputText("name", cfg_name, 64);
					ImGui::Separator();
					ImGui::SetCursorPos(ImVec2(5, 25 * 2.3));
					ImGui::Text("CONFIGS LIST");

					static auto should_update = true;

					if (should_update)
					{
						should_update = false;

						cfglist_update();

					}

					for (int c = 0; c < files.size(); ++c) {
						ImGui::SetCursorPos(ImVec2(5, (25 * 3) + 25 * c));
						if (ImGui::SubTab(files[c].c_str(), ImVec2(140, 25), config_edit == files[c].c_str() ? true : false))
							config_edit = files[c].c_str();
					}

					break;
					}
				}
			}
			ImGui::EndChild();

			ImGui::SetCursorPos(ImVec2{ 170, 50 });
			ImGui::BeginChild("##functional", ImVec2{ x - 180, y - 60 }, false);
			{
				ImGui::SetCursorPos(ImVec2{ 10, 10 });
				ImGui::BeginChild("##padding", ImVec2{ x - 200, y - 80 }, false);
				{
					switch (active_tab) {
						case 1: /*rage*/ {
							switch (active_subtab_rage) {
								case 0: {
									ImGui::Text("Aimbot functional");
									ImGui::Checkbox("Enabled##rage", g_Options.enabled_ragebot);
									ImGui::Checkbox("Autoscope##rage", g_Options.autoscope);

									break;
								}
								case 1: {
									ImGui::Text("Doubletap");
									ImGui::Checkbox("Doubletap", g_Options.doubletap);
									ImGui::SameLine();
									ImGui::Bind("##dt_bind", g_Options.doubletap_bind);

									break;
								}
								case 2: {


									break;
								}
								default: {
									const char* hitboxes[] = { "Head", "Upper hest", "Chest", "Lower chest", "Arms", "Legs", "Foots" };
									ImGui::Text("Weapon settings");


									ImGui::Separator();

									ImGui::Text("Hitboxes");

									ImGui::SelectableBox("Head", g_Options.Weapon[active_weapon_edit].head); ImGui::SameLine(); ImGui::SelectableBox("Chest", g_Options.Weapon[active_weapon_edit].chest);;
									ImGui::SelectableBox("Body", g_Options.Weapon[active_weapon_edit].body); ImGui::SameLine(); ImGui::SelectableBox("Arms", g_Options.Weapon[active_weapon_edit].arms);
									ImGui::SelectableBox("Legs", g_Options.Weapon[active_weapon_edit].legs); ImGui::SameLine(); ImGui::SelectableBox("Foots", g_Options.Weapon[active_weapon_edit].foot);

									ImGui::Separator();


									ImGui::SliderInt("Hitchance", g_Options.Weapon[active_weapon_edit].hitchance, 1, 100);
									ImGui::SliderInt("Multipoints", g_Options.Weapon[active_weapon_edit].multipoints, 0, 100);
									ImGui::Checkbox("Autostop", g_Options.Weapon[active_weapon_edit].autostop);
									ImGui::SliderInt("Damage", g_Options.Weapon[active_weapon_edit].damage, 1, 100);
									ImGui::Bind("Damage override bind", g_Options.Weapon[active_weapon_edit].damage_override_bind);
									ImGui::SliderInt("Damage override", g_Options.Weapon[active_weapon_edit].damage_override, 1, 100);
									ImGui::Checkbox("RCS", g_Options.Weapon[active_weapon_edit].rcs);

									break;
								}
							}
							break;
						}
						case 2: /*aa*/ {
							switch (active_subtab_aa) {
								case 0: {
									

									break;
								}
								case 1: {


									break;
								}
								case 2: {
									ImGui::Text("Animation changes");

									ImGui::Checkbox("Static legs", g_Options.staticLegs);
									ImGui::Checkbox("Slide walk", g_Options.slidewalk);

									break;
								}
							}
							break;
						}
						case 3: /*visual*/ {
							const char* C_styles[] = { "Flat", "Regular", "Glow"};

							switch (active_subtab_esp) {
								case 0: { /*enemy*/
									ImGui::Text("Esp settings");
									ImGui::Checkbox("Enabled##enemy", g_Options.enabled_esp);

									break;
								}
								case 1: { /*enemy chams*/
									ImGui::Checkbox("Enable chams on enemy", g_Options.chams_player_enabled);
									if (g_Options.chams_player_enabled) {
										ImGui::Checkbox("Through wall chams", g_Options.chams_player_ignorez);
										ImGui::Combo("Type", g_Options.chams_player_type, C_styles, IM_ARRAYSIZE(C_styles));
										ImGui::NewLine();
										ImGui::ColorEdit4("Color chams visible##enemy", g_Options.color_chams_player_enemy_visible);
										if (g_Options.chams_player_ignorez) {
											ImGui::NewLine();
											ImGui::ColorEdit4("Color chams through wall##enemy", g_Options.color_chams_player_enemy_occluded);
										}
									}

									break;
								}
								case 2: { /*local chams*/
									ImGui::Checkbox("Enable chams on arms", g_Options.chams_arms_enabled);
									if (g_Options.chams_arms_enabled) {
										ImGui::Combo("Type", g_Options.chams_arm_type, C_styles, IM_ARRAYSIZE(C_styles));
										ImGui::NewLine();
										ImGui::ColorEdit4("Color arms chams##arms", g_Options.color_chams_arms, false);
									}

									break;
								}
								case 3: {
									ImGui::Text("Indicators");
									ImGui::Checkbox("Watermark", g_Options.watermark);
									if (g_Options.watermark) {
										ImGui::SameLine();
										ImGui::ColorEdit4("Color##water", g_Options.color_watermark, false);
									}

									break;
								}
								case 4: {
									ImGui::Text("General");
									ImGui::Checkbox("Third Person", g_Options.thirdperson);
									if (g_Options.thirdperson)
									{
										ImGui::Bind("Bind##thirdperson", g_Options.thirdperson_bind);
										ImGui::SliderInt("Distance", g_Options.thirdperson_dist, 25, 150);
									}

									break;
								}
								case 5: { /*main*/

									ImGui::Text("Grenades");
									ImGui::Checkbox("Grenade Warning", g_Options.grenades);
									if (g_Options.grenades) {
										ImGui::SameLine();
										ImGui::ColorEdit4("Color##warning", g_Options.grenades_color, false);
									}

									break;
								}
								case 6: {
									ImGui::Text("Esp settings");
									ImGui::Checkbox("Weapons", g_Options.esp_dropped_weapons);
									ImGui::Checkbox("Planted c4 ESP", g_Options.esp_planted_c4);
									ImGui::Checkbox("Esp items draw", g_Options.esp_items);
									ImGui::Text("Removals");
									ImGui::Checkbox("Sniper crosshair", g_Options.sniper_crosshair);
									ImGui::Checkbox("Scope##remove", g_Options.scope_remove);

									break;
								}
								case 7: {
									

									break;
								}

							}
							
							break;
						}
						case 4: /*misc*/ {
							switch (active_subtab_misc) {
								case 0: {
									ImGui::Text("Misc functional");
									ImGui::SliderFloat("Aspect ratio", g_Options.misc_aspect_ratio, 0, 3);
									ImGui::Checkbox("Bunny hop", g_Options.bhop);
									break;
								}
								case 1: {


									break;
								}
								case 2: {
									

									break;
								}
							}
							break;
						}
						case 5: /*configs*/ {
							ImGui::Text("Configs system");
							
							if (ImGui::Button("Create cfg")) {
								Config::Get().Create(cfg_name);

								cfglist_update();
							}

							if (ImGui::Button("Reload configs")) {
								cfglist_update();
							}

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::Text("Config edit");

							if (config_edit != "") {
								if (config_active == config_edit) {
									if (ImGui::Button("Save config")) {
										Config::Get().Save(config_edit);
									}
								}
								else {
									ImGui::Text("You can't save the config when you have another one loaded");
								}
								
								if (ImGui::Button("Load config")) {
									Config::Get().Load(config_edit);
									config_active = config_edit;
								}
								
								ImGui::InputText("rename config", cfg_name_new, 64);
								if (ImGui::Button("Rename config")) {
									Config::Get().Rename(config_edit, cfg_name_new );

									cfglist_update();
								}

								if (ImGui::Button("Remove config")) {
									Config::Get().Remove(config_edit);

									cfglist_update();
								}
							}
							break;
						}
					}
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();
		}
		pos_esp_preview = ImGui::GetWindowPos();
    }
	ImGui::End();

	if (active_tab == 3 && g_Options.enabled_esp) {
		bool _esp_preview_vis = true;
		float esp_preview_x = 280;
		float esp_preview_y = y - 50 * 2;
		ImVec2 box_size = ImVec2(esp_preview_x - 100, 250);
		ImVec2 def_pos = ImVec2(50, 50);
		ImGui::SetNextWindowPos(ImVec2(pos_esp_preview.x + x + 20, pos_esp_preview.y + 50));
		ImGui::SetNextWindowSize(ImVec2(esp_preview_x, esp_preview_y));
		ImGui::Begin("Esp preview",
			&_esp_preview_vis,
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove); {

			auto cur_window = ImGui::GetCurrentWindow();
			ImVec2 w_pos = cur_window->Pos;

			
			//name draw
			if (g_Options.esp_player_names) {
				if (ImGui::BeginPopup("Names", ImGuiWindowFlags_NoMove)) {

					//

					ImGui::EndPopup();
				}
				auto size_text_name = ImGui::CalcTextSize("name");
				ImGui::SetCursorPos(ImVec2(def_pos.x + box_size.x / 2 - size_text_name.x / 2, 35));
				ImGui::Popup("Names", ImVec2(size_text_name.x, size_text_name.y));
				cur_window->DrawList->AddText(ImVec2(w_pos.x + def_pos.x + box_size.x / 2 - size_text_name.x / 2, w_pos.y + 35), ImColor(255, 255, 255, 255), "name");
			}

			//weapon draw
			if (g_Options.esp_player_weapons) {
				if (ImGui::BeginPopup("Weapon", ImGuiWindowFlags_NoMove)) {

					//

					ImGui::EndPopup();
				}
				auto size_text_name = ImGui::CalcTextSize("ak-47");
				ImGui::SetCursorPos(ImVec2(def_pos.x + box_size.x / 2 - size_text_name.x / 2, 310));
				ImGui::Popup("Weapon", ImVec2(size_text_name.x, size_text_name.y));
				cur_window->DrawList->AddText(ImVec2(w_pos.x + def_pos.x + box_size.x / 2 - size_text_name.x / 2, w_pos.y + 310), ImColor(255, 255, 255, 255), "ak-47");
			}

			//box draw
			if (g_Options.esp_player_boxes) {
				if (ImGui::BeginPopup("Box", ImGuiWindowFlags_NoMove)) {

					ImGui::Checkbox("Outline##box", g_Options.esp_player_boxes_outline);

					ImGui::EndPopup();
				}
				ImGui::SetCursorPos(ImVec2(def_pos.x, def_pos.y));
					ImGui::Popup("Box", ImVec2(box_size.x, 2));
				ImGui::SetCursorPos(ImVec2(def_pos.x, def_pos.y));
					ImGui::Popup("Box", ImVec2(2, box_size.y));
				ImGui::SetCursorPos(ImVec2(def_pos.x, def_pos.y + box_size.y));
					ImGui::Popup("Box", ImVec2(box_size.x, 2));
				ImGui::SetCursorPos(ImVec2(def_pos.x + box_size.x, def_pos.y));
					ImGui::Popup("Box", ImVec2(2, box_size.y));

				cur_window->DrawList->AddRect(ImVec2(w_pos.x + def_pos.x, w_pos.y + def_pos.y), ImVec2(w_pos.x + def_pos.x + box_size.x, w_pos.y + def_pos.y + box_size.y), ImColor(255, 255, 255), 0, 15, 2);
				if (g_Options.esp_player_boxes_outline) {
					cur_window->DrawList->AddRect(ImVec2(w_pos.x + def_pos.x - 1, w_pos.y + def_pos.y - 1), ImVec2(w_pos.x + def_pos.x + 1 + box_size.x, w_pos.y + def_pos.y + 1 + box_size.y), ImColor(0, 0, 0));
					cur_window->DrawList->AddRect(ImVec2(w_pos.x + def_pos.x + 2, w_pos.y + def_pos.y + 3), ImVec2(w_pos.x + def_pos.x - 2 + box_size.x, w_pos.y + def_pos.y - 2 + box_size.y), ImColor(0, 0, 0));
				}
			}

			//hp bar draw
			if (g_Options.esp_player_health) {
				if (ImGui::BeginPopup("Healthbar", ImGuiWindowFlags_NoMove)) {

					ImGui::Checkbox("Black HP line##healthbar", g_Options.esp_player_hp_line);
					ImGui::NewLine();
					ImGui::ColorEdit4("Color healthbar##healthbar", g_Options.esp_player_hp_health, false);

					ImGui::EndPopup();	
				}
				ImGui::SetCursorPos(ImVec2(def_pos.x - 10, def_pos.y));
				ImGui::Popup("Healthbar", ImVec2(4, box_size.y));
				cur_window->DrawList->AddRectFilled(ImVec2(w_pos.x + def_pos.x - 10, w_pos.y + def_pos.y), ImVec2(w_pos.x + def_pos.x - 6, w_pos.y + def_pos.y + box_size.y), ImColor(255, 255, 255));
				cur_window->DrawList->AddRect(ImVec2(w_pos.x + def_pos.x - 10, w_pos.y + def_pos.y), ImVec2(w_pos.x + def_pos.x - 6, w_pos.y + def_pos.y + box_size.y), ImColor(0, 0, 0));
			}

			//armor draw
			if (g_Options.esp_player_armour) {
				if (ImGui::BeginPopup("Armorbar", ImGuiWindowFlags_NoMove)) {

					ImGui::ColorEdit4("Color armor bar##enemy", g_Options.esp_player_armour_color, false);

					ImGui::EndPopup();
				}
				ImGui::SetCursorPos(ImVec2(def_pos.x + box_size.x + 6, def_pos.y));
				ImGui::Popup("Armorbar", ImVec2(4, box_size.y));
				cur_window->DrawList->AddRectFilled(ImVec2(w_pos.x + def_pos.x + box_size.x + 6, w_pos.y + def_pos.y), ImVec2(w_pos.x + def_pos.x + box_size.x + 10, w_pos.y + def_pos.y + box_size.y), ImColor(100, 145, 230));
				cur_window->DrawList->AddRect(ImVec2(w_pos.x + def_pos.x + box_size.x + 6, w_pos.y + def_pos.y), ImVec2(w_pos.x + def_pos.x + box_size.x + 10, w_pos.y + def_pos.y + box_size.y), ImColor(0, 0, 0));
			}

			//ammo draw
			if (g_Options.esp_player_ammo) {
				if (ImGui::BeginPopup("Ammobar", ImGuiWindowFlags_NoMove)) {

					ImGui::ColorEdit4("Color ammo##enemy", g_Options.esp_player_ammo_color, false);

					ImGui::EndPopup();
				}
				ImGui::SetCursorPos(ImVec2(def_pos.x, def_pos.y + box_size.y + 5));
				ImGui::Popup("Ammobar", ImVec2(box_size.x, 4));
				cur_window->DrawList->AddRectFilled(ImVec2(w_pos.x + def_pos.x, w_pos.y + def_pos.y + box_size.y + 5), ImVec2(w_pos.x + def_pos.x + box_size.x, w_pos.y + def_pos.y + box_size.y + 9), ImColor(60, 60, 255));
				cur_window->DrawList->AddRect(ImVec2(w_pos.x + def_pos.x, w_pos.y + def_pos.y + box_size.y + 5), ImVec2(w_pos.x + def_pos.x + box_size.x, w_pos.y + def_pos.y + box_size.y + 9), ImColor(0, 0, 0));
			}


			ImGui::SetCursorPos(ImVec2(0, esp_preview_y - 105));
			ImGui::BeginChild("##esp flags", ImVec2{ esp_preview_x, 95 }, false);
			{

				ImGui::SetCursorPos(ImVec2(0, 0));
				gor_line(esp_preview_x, ImColor(39, 39, 39));

				ImGui::SetCursorPos(ImVec2(5, 5));
				ImGui::BeginChild("##flags", ImVec2{ esp_preview_x - 10, 85 }, false);
				{
					ImGui::SelectableBox("Name", g_Options.esp_player_names);
					ImGui::SameLine();
					ImGui::SelectableBox("Box", g_Options.esp_player_boxes);
					ImGui::SameLine();
					ImGui::SelectableBox("Health bar", g_Options.esp_player_health);
					ImGui::SameLine();
					ImGui::SelectableBox("Armor bar", g_Options.esp_player_armour);
					
					ImGui::SelectableBox("Weapons", g_Options.esp_player_weapons);
					ImGui::SameLine();
					ImGui::SelectableBox("Ammo", g_Options.esp_player_ammo);
				}
				ImGui::EndChild();
				     
			}
			ImGui::EndChild();

		}
		ImGui::End();
	}
}

void Menu::Toggle()
{
    _visible = !_visible;
}

void Menu::CreateStyle()
{
	ImGui::StyleColorsDark();
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
	_style.FrameRounding = 0.f;
	_style.WindowRounding = 0.f;
	_style.ChildRounding = 0.f;
	_style.Colors[ImGuiCol_Button] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_Header] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.260f, 0.590f, 0.980f, 1.000f);
	//_style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.000f, 0.545f, 1.000f, 1.000f);
	//_style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.060f, 0.416f, 0.980f, 1.000f);
	_style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.30f, 1.0f);
	_style.Colors[ImGuiCol_WindowBg] = ImVec4(0.000f, 0.009f, 0.120f, 0.940f);
	_style.Colors[ImGuiCol_PopupBg] = ImVec4(0.076f, 0.143f, 0.209f, 1.000f);
	ImGui::GetStyle() = _style;
}

