#include "../valve_sdk/csgostructs.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"
#include "../helpers/math.hpp"

class Misc : public Singleton<Misc>
{
public:

	void Bhop(CUserCmd* cmd);

};