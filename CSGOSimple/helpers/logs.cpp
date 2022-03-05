#include "logs.hpp"
#include "../render.hpp"

void Logs::Create(const char* text)
{
	if (!g_Options.logs)
		return;

	logs.emplace_front(loginfo_t(Utils::epoch_time(), text, g_Options.menu_color));

	g_CVar->ConsoleColorPrintf(Color::White, "[");
	g_CVar->ConsoleColorPrintf(Color(g_Options.menu_color.r(), g_Options.menu_color.g(), g_Options.menu_color.b()), "nessless");
	g_CVar->ConsoleColorPrintf(Color::White, "] ");
	g_CVar->ConsoleColorPrintf(Color::White, text);
	g_CVar->ConsolePrintf("\n");
}

void Logs::Draw()
{
	if (!g_Options.logs_drawing)
		return;

	if (logs.empty())
		return;

	while (logs.size() > 10)
		logs.pop_back();

	auto last_y = 146;

	for (size_t i = 0; i < logs.size(); i++)
	{
		auto& log = logs.at(i);

		if (Utils::epoch_time() - log.log_time > 4600)
		{
			auto factor = log.log_time + 5000.0f - (float)Utils::epoch_time();
			factor *= 0.001f;

			auto opacity = (int)(255.0f * factor);

			if (opacity < 2)
			{
				logs.erase(logs.begin() + i);
				continue;
			}

			log.color = Color(log.color.r(), log.color.g(), log.color.b(), opacity);
			log.y -= factor * 1.25f;
		}

		last_y -= 14;

		auto logs_size_inverted = 10 - logs.size();
		Render::Get().RenderText("*", log.x, last_y + log.y - logs_size_inverted * 14, 14.f, log.color, false, true, g_MenuFont);
		Render::Get().RenderText(log.message.c_str(), log.x + 4, last_y + log.y - logs_size_inverted * 14, 14.f, Color::White, false, true, g_MenuFont);
	}
}