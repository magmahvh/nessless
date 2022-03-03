#include "resolver.h"
#include "../options.hpp"
#include "../helpers/math.hpp"

void Resolver::Resolve(C_BasePlayer* player) {
    if (player->GetPlayerAnimState())
    {
        Vector vVelocity = player->m_vecVelocity();
        float flSpeed = vVelocity.Length2D();

        if (flSpeed == 0.0f)
        {
            if (g_Options.rage_enabled)
            {
                Vector vEyeAngles = player->GetEyePos();
                float flLowerBody = remainderf(player->m_flLowerBodyYawTarget(), 360.f);

                if (flLowerBody - remainderf(vEyeAngles.y, 360.f) >= 60.f)
                    player->GetPlayerAnimState()->m_flGoalFeetYaw = (rand() % 50 + 1);
            }
        }
    }
}