#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"
#include "antiaim.h"

void AntiAim::slidewalk(CUserCmd* cmd) {
    if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;

    if (cmd->forwardmove > 0.f) {
        cmd->buttons |= IN_BACK;
        cmd->buttons &= ~IN_FORWARD;
    }
    else if (cmd->forwardmove < 0.f) {
        cmd->buttons |= IN_FORWARD;
        cmd->buttons &= ~IN_BACK;
    }
    if (cmd->sidemove > 0.f) {
        cmd->buttons |= IN_MOVELEFT;
        cmd->buttons &= ~IN_MOVERIGHT;
    }
    else if (cmd->sidemove < 0.f) {
        cmd->buttons |= IN_MOVERIGHT;
        cmd->buttons &= ~IN_MOVELEFT;
    }
}