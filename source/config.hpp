#pragma once
#include "options.hpp"
#include "singleton.hpp"
#include <tchar.h>
#include <fstream>
#include "user.h"




class Config : public Singleton<Config> {
public:
	void Create(std::string cfg_name) {
		std::ofstream fout(cfg_name + ".cfg", std::ios::binary);
		const auto sz = sizeof(Options);
		const auto var_sz = sizeof(Var<bool>);
		const auto cnt = sz / var_sz;
		for (auto i = 0; i < cnt; i++) {
			const auto el = &(*(Var<int>*)(&g_Options)) + i;
			auto name = el->name;
			auto val = el->value;
			auto sizeof_val = el->size;
			fout << name << "\t" << Utils::BytesToString((unsigned char*)*(int*)&val, sizeof_val) << std::endl;
		}
		fout.close();
	}

	void Save(std::string cfg_name) {
		std::ofstream fout(cfg_name, std::ios::binary);
		const auto sz = sizeof(Options);
		const auto var_sz = sizeof(Var<bool>);
		const auto cnt = sz / var_sz;
		for (auto i = 0; i < cnt; i++) {
			const auto el = &(*(Var<int>*)(&g_Options)) + i;
			auto name = el->name;
			auto val = el->value;
			auto sizeof_val = el->size;
			fout << name << "\t" << Utils::BytesToString((unsigned char*)*(int*)&val, sizeof_val) << std::endl;
		}
		fout.close();
	}


	void Load(std::string cfg_name) {
		std::ifstream fin(cfg_name, std::ios::binary);
		std::stringstream ss;
		ss << fin.rdbuf();


		auto lines = Utils::Split(ss.str(), "\n");

		for (auto line : lines) {
			auto data = Utils::Split(line, "\t");
			const auto sz = sizeof(Options);
			const auto var_sz = sizeof(Var<bool>);
			const auto cnt = sz / var_sz;
			for (auto i = 0; i < cnt; i++) {
				const auto &el = &(*(Var<bool>*)(&g_Options)) + i;
				if (data[0] == el->name) {
					auto bytes = Utils::HexToBytes(data[1]);
					memcpy(*(void**)&el->value, bytes.data(), el->size);
				}
			}
		}
		fin.close();
	}

	void Remove(std::string cfg_name)
	{
		std::string path = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\" + cfg_name;
		remove(path.c_str());
	}

	void Rename(std::string cfg_name_old, std::string cfg_name_new) {
		std::string old_name = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\" + cfg_name_old;
		std::string new_name = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\" + cfg_name_new + ".cfg";
		rename(old_name.c_str(), new_name.c_str());
	}

	std::vector<std::string> items = {};
	void Files()
	{
		items.clear();
		std::string path = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive"; // Тут изменяем ( \\ после текста нинада)

		for (auto& p : std::filesystem::directory_iterator(path)) {
			std::string all_files = p.path().string().substr(path.length() + 1);
			if (all_files.find(".cfg") != std::string::npos)
				items.push_back(all_files);
		}
	}
};