#include "../singleton.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include <windows.h>
#include <Lmcons.h>

class Cheat : public Singleton<Cheat> {
public:

	std::string username = g_CVar->FindVar("name")->GetString();
};