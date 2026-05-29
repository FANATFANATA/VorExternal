#pragma once
#include "../dump/offsets.hpp"
#include "../dump/client_dll.hpp"
#include "../dump/engine2_dll.hpp"

namespace game
{
    namespace offsets
    {
        constexpr auto dwLocalPlayerPawn = cs2_dumper::offsets::client_dll::dwLocalPlayerPawn;
        constexpr auto dwEntityList = cs2_dumper::offsets::client_dll::dwEntityList;
        constexpr auto dwViewMatrix = cs2_dumper::offsets::client_dll::dwViewMatrix;
    }

    namespace fields
    {
        constexpr auto m_hPlayerPawn = cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn;
        constexpr auto m_bPawnIsAlive = cs2_dumper::schemas::client_dll::CCSPlayerController::m_bPawnIsAlive;
        constexpr auto m_iszPlayerName = cs2_dumper::schemas::client_dll::CBasePlayerController::m_iszPlayerName;
        constexpr auto m_iHealth = cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth;
        constexpr auto m_iTeamNum = cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum;
        constexpr auto m_vOldOrigin = cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin;
    }
}
