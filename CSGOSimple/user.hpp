
#include <windows.h>
#include <Lmcons.h>

class Cheat : public Singleton<Cheat> {
public:

	std::string username = g_CVar->FindVar("name")->GetString();
};