#include "world.h"
#include "../options.hpp"
#include "../valve_sdk/interfaces/IMaterialSystem.hpp"
std::vector <MaterialBackup> materials;
bool changed = false;
std::string backup_skybox = "";

void World::clear_stored_materials()
{
	materials.clear();
}

void World::modulate(MaterialHandle_t i, IMaterial* material, bool backup = false)
{
	auto name = material->GetTextureGroupName();

	if (strstr(name, "World"))
	{
		if (backup)
			materials.emplace_back(MaterialBackup(i, material));
		if (g_Options.enable_nightmode)
			material->ColorModulate((float)g_Options.nightmode_color.r() / 255.0f, (float)g_Options.nightmode_color.g() / 255.0f, (float)g_Options.nightmode_color.b() / 255.0f);
		else
			material->ColorModulate(1.f, 1.f, 1.f);
	}
	else if (strstr(name, "StaticProp") || strstr(name, "Prop"))
	{
		if (backup)
			materials.emplace_back(MaterialBackup(i, material));
		if (g_Options.enable_nightmode)
			material->ColorModulate(((float)g_Options.nightmode_color.r() / 255.0f) * 0.8f, ((float)g_Options.nightmode_color.g() / 255.0f) * 0.8f, ((float)g_Options.nightmode_color.b() / 255.0f) * 0.8f);
		else
			material->ColorModulate(1.f, 1.f, 1.f);
	}
}
void World::modulate_transp(MaterialHandle_t i, IMaterial* material, bool backup = false)
{
	auto name = material->GetTextureGroupName();
	if (strstr(name, "StaticProp"))
	{
		if (backup)
			materials.emplace_back(MaterialBackup(i, material));
		material->AlphaModulate((float)1 - g_Options.asus_props);
	}
	else if (strstr(name, "World"))
	{
		if (backup)
			materials.emplace_back(MaterialBackup(i, material));
		material->AlphaModulate((float)1 - g_Options.asus_walls);
	}
}
void World::apply()
{
	if (!materials.empty())
	{
		for (auto i = 0; i < (int)materials.size(); i++)
			modulate(materials[i].handle, materials[i].material);

		return;
	}
	materials.clear();
	auto materialsystem = g_MatSystem;
	for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
	{
		auto material = materialsystem->GetMaterial(i);
		if (!material)
			continue;
		if (material->IsErrorMaterial())
			continue;
		modulate(i, material, true);
	}
}

void World::asus()
{
	if (!materials.empty())
	{
		for (auto i = 0; i < (int)materials.size(); i++)
			modulate_transp(materials[i].handle, materials[i].material);
		return;
	}
	materials.clear();
	auto materialsystem = g_MatSystem;

	for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
	{
		auto material = materialsystem->GetMaterial(i);
		if (!material)
			continue;
		if (material->IsErrorMaterial())
			continue;
		modulate_transp(i, material, true);
	}
}

void World::remove()
{
	for (auto i = 0; i < materials.size(); i++)
	{
		if (!materials[i].material)
			continue;

		if (materials[i].material->IsErrorMaterial())
			continue;

		materials[i].restore();
		materials[i].material->Refresh();
	}

	materials.clear();
}

void World::skybox_changer()
{
	static auto fnLoadNamedSkys = (void(__fastcall*)(const char*))Utils::PatternScan(GetModuleHandleA("engine.dll"), "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45");
	auto skybox_name = backup_skybox;

	switch (g_Options.skybox_num)
	{
	case 1:
		skybox_name = "cs_tibet";
		break;
	case 2:
		skybox_name = "cs_baggage_skybox_";
		break;
	case 3:
		skybox_name = "italy";
		break;
	case 4:
		skybox_name = "jungle";
		break;
	case 5:
		skybox_name = "office";
		break;
	case 6:
		skybox_name = "sky_cs15_daylight01_hdr";
		break;
	case 7:
		skybox_name = "sky_cs15_daylight02_hdr";
		break;
	case 8:
		skybox_name = "vertigoblue_hdr";
		break;
	case 9:
		skybox_name = "vertigo";
		break;
	case 10:
		skybox_name = "sky_day02_05_hdr";
		break;
	case 11:
		skybox_name = "nukeblank";
		break;
	case 12:
		skybox_name = "sky_venice";
		break;
	case 13:
		skybox_name = "sky_cs15_daylight03_hdr";
		break;
	case 14:
		skybox_name = "sky_cs15_daylight04_hdr";
		break;
	case 15:
		skybox_name = "sky_csgo_cloudy01";
		break;
	case 16:
		skybox_name = "sky_csgo_night02";
		break;
	case 17:
		skybox_name = "sky_csgo_night02b";
		break;
	case 18:
		skybox_name = "sky_csgo_night_flat";
		break;
	case 19:
		skybox_name = "sky_dust";
		break;
	case 20:
		skybox_name = "vietnam";
		break;
	}

	static auto skybox_number = 0;
	static auto old_skybox_name = skybox_name;
	static auto color_r = (unsigned char)255;
	static auto color_g = (unsigned char)255;
	static auto color_b = (unsigned char)255;


	if (skybox_number != g_Options.skybox_num)
	{
		changed = true;
		skybox_number = g_Options.skybox_num;
	}
	else if (old_skybox_name != skybox_name)
	{
		changed = true;
		old_skybox_name = skybox_name;
	}
	else if (color_r != g_Options.skybox_color.r())
	{
		changed = true;
		color_r = g_Options.skybox_color.r();
	}
	else if (color_g != g_Options.skybox_color.g())
	{
		changed = true;
		color_g = g_Options.skybox_color.g();
	}
	else if (color_b != g_Options.skybox_color.b())
	{
		changed = true;
		color_b = g_Options.skybox_color.b();
	}

	if (changed)
	{
		changed = false;
		fnLoadNamedSkys(skybox_name.c_str());
		auto materialsystem = g_MatSystem;

		for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
		{
			auto material = materialsystem->GetMaterial(i);

			if (!material)
				continue;

			if (strstr(material->GetTextureGroupName(), "SkyBox"))
				material->ColorModulate(g_Options.skybox_color.r() / 255.f, g_Options.skybox_color.g() / 255.f, g_Options.skybox_color.b() / 255.f);
		}
	}
}

void World::fog_changer()
{
	static auto fog_override = g_CVar->FindVar("fog_override");

	if (!g_Options.enable_fog)
	{
		if (fog_override->GetBool())
			fog_override->SetValue(FALSE);
		return;
	}

	if (!fog_override->GetBool())
		fog_override->SetValue(TRUE);

	static auto fog_start = g_CVar->FindVar("fog_start");

	if (fog_start->GetInt() != g_Options.fog_start_distance)
		fog_start->SetValue(g_Options.fog_start_distance);

	static auto fog_end = g_CVar->FindVar("fog_end");

	if (fog_end->GetInt() != g_Options.fog_end_distance)
		fog_end->SetValue(g_Options.fog_end_distance);

	static auto fog_maxdensity = g_CVar->FindVar("fog_maxdensity");

	if (fog_maxdensity->GetFloat() != (float)g_Options.fog_density * 0.01f)
		fog_maxdensity->SetValue((float)g_Options.fog_density * 0.01f);

	char buffer_color[12];
	sprintf_s(buffer_color, 12, "%i %i %i", g_Options.fog_color.r(), g_Options.fog_color.g(), g_Options.fog_color.b());

	static auto fog_color = g_CVar->FindVar("fog_color");

	if (strcmp(fog_color->GetString(), buffer_color))
		fog_color->SetValue(buffer_color);
}