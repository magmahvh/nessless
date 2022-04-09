#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../singleton.hpp"
#include "../options.hpp"

class FakeLag : public Singleton<FakeLag>
{
public:
    void Override(CUserCmd* cmd, bool& bSendPacket, int amount);
    void Run(CUserCmd* cmd, bool& bSendPacket);
};