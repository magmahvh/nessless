#pragma once

#include "..//options.hpp"
#include "..//valve_sdk/csgostructs.hpp"

class Misc : public Singleton<Misc>
{
public:
	void ThirdPerson();
	void Fakeduck(CUserCmd* cmd, bool& bSendPacket, int amount);
};