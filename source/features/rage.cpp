#include "rage.h"
int wpnGroup(CHandle<C_BaseCombatWeapon> pWeapon) {

    if (!pWeapon)
        return -1;

    if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SCAR20)
        return 0;
    else if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP)
        return 1;
    else if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08)
        return 2;
    else if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_DEAGLE || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
        return 3;
    else if (pWeapon->IsRifle())
        return 4;
    else if (pWeapon->IsPistol())
        return 5;
    else 
        return 6;
}

std::vector<target_selection_info> Entities;
namespace Entity {
    void TRGT_SELECTION()
    {

        for (int i = 1; i <= g_GlobalVars->maxClients; i++)
        {
            auto entity = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
            if (!entity) continue;

            if (!entity->IsValidTarget())
                continue;


            target_selection_info data = target_selection_info(entity, i);


            Entities.push_back(data);
        }
    }
}
namespace CanShoot {
    float GetCurtime()
    {
        if (!g_LocalPlayer)
            return -1;

        return static_cast<float>(g_LocalPlayer->m_nTickBase()) * g_GlobalVars->interval_per_tick;
    }

    bool Get(CUserCmd* cmd)
    {

        if (!g_LocalPlayer) return false;

        if (!g_LocalPlayer->m_hActiveWeapon().Get()) return false;

        return (g_LocalPlayer->m_hActiveWeapon().Get()->m_flNextPrimaryAttack() < GetCurtime()) && (g_LocalPlayer->m_flNextAttack() < GetCurtime());

    }
}

void Rage::Aim(CUserCmd* cmd) {
    if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive())  return;

    auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
    int wpn = wpnGroup(pWeapon);

    if (!pWeapon) return;
    if (pWeapon->m_iClip1() == 0) return;
    if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_C4 || pWeapon->IsGrenade() || pWeapon->IsKnife()) return;
    if (!CanShoot::Get(cmd)) { cmd->buttons &= ~IN_ATTACK;	return; }

    Entities.clear(); 
    Entity::TRGT_SELECTION();

    for (auto players : Entities)
    {
        auto entity = players.entity;

		

    }

    g_CVar->ConsolePrintf(std::to_string(wpn).c_str());
    g_CVar->ConsolePrintf("\n");
}