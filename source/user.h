#include "../singleton.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"

class Cheat : public Singleton<Cheat> {
public:
	std::string username = "admin";
};