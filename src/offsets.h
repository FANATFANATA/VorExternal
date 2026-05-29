#pragma once

#include <cstddef>

namespace offsets
{
    constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x233F698;
    constexpr std::ptrdiff_t dwEntityList = 0x24E5590;
    constexpr std::ptrdiff_t dwViewMatrix = 0x2344B30;
    constexpr std::ptrdiff_t dwViewAngles = 0x23548B8;
    constexpr std::ptrdiff_t dwLocalPlayerController = 0x231E700;

    constexpr std::ptrdiff_t m_pGameSceneNode = 0x330;
    constexpr std::ptrdiff_t m_pBoneArray = 0x1F0;

    constexpr std::ptrdiff_t m_hPlayerPawn = 0x90C;
    constexpr std::ptrdiff_t m_bPawnIsAlive = 0x914;
    constexpr std::ptrdiff_t m_iszPlayerName = 0x6F4;

    constexpr std::ptrdiff_t m_iHealth = 0x34C;
    constexpr std::ptrdiff_t m_iTeamNum = 0x3EB;
    constexpr std::ptrdiff_t m_vOldOrigin = 0x1390;

    constexpr std::ptrdiff_t m_iIDEntIndex = 0x33FC;
    constexpr std::ptrdiff_t m_iShotsFired = 0x1C64;
    constexpr std::ptrdiff_t m_pAimPunchServices = 0x1490;
    constexpr std::ptrdiff_t m_vecViewOffset = 0xE70;

    constexpr std::ptrdiff_t m_predictableBaseAngle = 0x50;

    constexpr std::ptrdiff_t dwNetworkGameClient = 0x90A1A0;
    constexpr std::ptrdiff_t dwNetworkGameClient_localPlayer = 0xF8;

    constexpr std::ptrdiff_t m_entitySpottedState = 0x1C38;
    constexpr std::ptrdiff_t m_bSpottedByMask = 0xC;
}
