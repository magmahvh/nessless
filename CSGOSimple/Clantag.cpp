#pragma once
#include "helpers/utils.hpp"
#include "options.hpp"
#include "Clantag.h"

void ClantagChanger::ClantagChange()
{
	static bool changed_clan = false;
	static int clantag = 0;
	if (g_Options.misc_combo_clantag != clantag) {
		changed_clan = false;
	}
	if (!changed_clan) {
		switch (g_Options.misc_combo_clantag) {
		case 0:
			Utils::SetClantag("");
			break;
		case 1:
			Utils::SetClantag("Nessles");
			break;
		}
		changed_clan = true;
		clantag = g_Options.misc_combo_clantag;
	}
}
