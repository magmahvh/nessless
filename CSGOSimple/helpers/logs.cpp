#include "logs.hpp"
#include "../render.hpp"

void Logs::Create(std::string text)
{
	if (!g_Options.logs)
		return;

	logs.emplace_front(loginfo_t(Utils::epoch_time(), text));

	g_CVar->ConsolePrintf("[desolate] "); 
	g_CVar->ConsolePrintf(text.c_str());
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

	auto last_y = -20;

	for (size_t i = 0; i < logs.size(); i++)
	{
		auto& log = logs.at(i);
		int opacity;

		if (Utils::epoch_time() - log.log_time > 4600)
		{
			auto factor = log.log_time + 5000.0f - (float)Utils::epoch_time();
			factor *= 0.001f;

			opacity = (int)(255.0f * factor);

			if (opacity < 2)
			{
				logs.erase(logs.begin() + i);
				continue;
			}

			log.y -= factor * 1.25f;
		}

		last_y += 35;
		
		auto y = last_y + log.y;
		auto x = log.x;

		Render::Get().RenderBoxFilled(x, y, x + Render::Get().GetTextSize(log.message.c_str(), 14) + 20, y + 30, Color(15, 15, 15, 170), 3);
		Render::Get().RenderBoxFilled(x, y, x + 3, y + 30, g_Options.menu_color, 0);
		Render::Get().RenderText(log.message.c_str(), x + 10, y + 7, 14.f, Color::White, false, false, g_MenuFont);
	}
}