#include "render.hpp"

#include <mutex>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "fonts/fonts.hpp"
#include "helpers/math.hpp"
#include "features/grenades.h"
#include "features/visuals.h"

ImFont* g_pDefaultFont;
ImFont* g_pSecondFont;

ImDrawListSharedData _data;

std::mutex render_mutex;

void Render::Initialize()
{
	ImGui::CreateContext();


	ImGui_ImplWin32_Init(InputSys::Get().GetMainWindow());
	ImGui_ImplDX9_Init(g_D3DDevice9);

	draw_list = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_act = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_rendering = new ImDrawList(ImGui::GetDrawListSharedData());

	GetFonts();
}

void Render::GetFonts() {

	// menu font
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Droid_compressed_data,
		Fonts::Droid_compressed_size,
		14.f);
	
	// esp font
	g_pDefaultFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Droid_compressed_data,
		Fonts::Droid_compressed_size,
		18.f);
	

	// font for watermark; just example
	g_pSecondFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Cousine_compressed_data,
		Fonts::Cousine_compressed_size,
		18.f); 
}

void Render::ClearDrawList() {
	render_mutex.lock();
	draw_list_act->Clear();
	render_mutex.unlock();
}

int get_fps()
{
	using namespace std::chrono;
	static int count = 0;
	static auto last = high_resolution_clock::now();
	auto now = high_resolution_clock::now();
	static int fps = 0;

	count++;

	if (duration_cast<milliseconds>(now - last).count() > 1000) {
		fps = count;
		count = 0;
		last = now;
	}

	return fps;
}

std::vector<std::string> m_keybinds;
void Render::BeginScene() {
	draw_list->Clear();
	draw_list->PushClipRectFullScreen();

	if (g_Options.watermark) {
		std::string water_text = "OTC3 addon";
		Render::Get().RenderText(water_text, 10, 7, 12.f, Color(255, 255, 255, 255), false, false, g_pSecondFont);
		int size = g_pSecondFont->CalcTextSizeA(12, FLT_MAX, 0.0f, water_text.c_str()).x;
		Render::Get().RenderBoxFilled(7, 5, size + 10, 2, g_Options.color_watermark);
	}

	/*bool _kb_enable = true;
	if (g_Options.keybinds) {

		if (GetKeyState(g_Options.thirdperson_bind))
			m_keybinds.emplace_back("Third person");

		ImGui::Begin("Keybinds",
			&_kb_enable,
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoTitleBar); 
		{
			auto cur_window = ImGui::GetCurrentWindow();
			ImVec2 w_pos = cur_window->Pos;
			int w_pad = 17;
			int x_pad = 150;
			ImGui::SetWindowSize(ImVec2(w_pad, x_pad));
			ImVec2 w_size = ImGui::GetWindowSize();

			cur_window->DrawList->AddRectFilled(ImVec2(w_pos.x, w_pos.y), ImVec2(w_pos.x + w_size.x, w_pos.y + 17), ImColor(20, 20, 20, 255));
			cur_window->DrawList->AddText(ImVec2(w_pos.x + 5, w_pos.y + 3), ImColor(255, 255, 255, 255), "keybinds");

			for (auto& keybind : m_keybinds) {
				cur_window->DrawList->AddText(ImVec2(w_pos.x + 5, w_pos.y + w_pad), ImColor(255, 255, 255, 255), keybind.c_str());
				w_pad += 17;
			}

		}
		ImGui::End();
	}*/

	if (g_EngineClient->IsInGame() && g_LocalPlayer)
		Visuals::Get().AddToDrawList();

	if (g_EngineClient->IsInGame())
	{

		for (auto k = 0; k < g_EntityList->GetHighestEntityIndex(); k++)
		{
			C_BasePlayer* entity = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(k));

			if (entity == nullptr ||
				!entity->GetClientClass() ||
				entity == g_LocalPlayer)
				continue;

			c_grenade_prediction::Get().grenade_warning(entity);
			c_grenade_prediction::Get().get_local_data().draw();
		}

	}

	render_mutex.lock();
	*draw_list_act = *draw_list;
	render_mutex.unlock();
}

ImDrawList* Render::RenderScene() {

	if (render_mutex.try_lock()) {
		*draw_list_rendering = *draw_list_act;
		render_mutex.unlock();
	}

	return draw_list_rendering;
}


float Render::RenderText(const std::string& text, ImVec2 pos, float size, Color color, bool center, bool outline, ImFont* pFont)
{
	ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());
	if (!pFont->ContainerAtlas) return 0.f;
	draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

	if (center)
		pos.x -= textSize.x / 2.0f;

	if (outline) {
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
	}

	draw_list->AddText(pFont, size, pos, GetU32(color), text.c_str());

	draw_list->PopTextureID();

	return pos.y + textSize.y;
}

void Render::RenderCircle3D(Vector position, float points, float radius, Color color)
{
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
	{
		Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
		Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

		Vector start2d, end2d;
		if (g_DebugOverlay->ScreenPosition(start, start2d) || g_DebugOverlay->ScreenPosition(end, end2d))
			return;

		RenderLine(start2d.x, start2d.y, end2d.x, end2d.y, color);
	}
}