#pragma once

class C_BasePlayer;
class CUserCmd;

namespace BunnyHop
{
    void OnCreateMove(CUserCmd* cmd);
    void AutoStrafe(CUserCmd* cmd);

}