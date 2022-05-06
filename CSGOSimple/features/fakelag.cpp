#include "fakelag.hpp"
#include "../helpers/math.hpp"

#define TIME_TO_TICKS(time) ((int)(0.5 +  float(time) / g_GlobalVars->interval_per_tick))

void FakeLag::Override(CUserCmd* cmd, bool& bSendPacket, int amount) {
    static int ticks = 0;
    int ticksMax = 16;

    if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
        return;

    if (cmd->buttons & IN_ATTACK)
    {
        bSendPacket = true;
        return;
    }

    if (ticks >= ticksMax)
    {
        bSendPacket = true;
        ticks = 0;
    }
    else
        bSendPacket = ticks < 16 - amount;

    ticks++;
}

void FakeLag::Run(CUserCmd* cmd, bool& bSendPacket)
{
    if (!g_Options.fakelag) return;

    FakeLag::Get().Override(cmd, bSendPacket, g_Options.fakelag_amount);
}


