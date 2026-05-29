#include "types.h"
#include "math/math.h"
#include "config/config.h"
#include "cheat/cheat.h"
#include "render/render.h"
#include "memory/memory.h"
#include "overlay/overlay.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>

namespace consts
{
    constexpr float HEAD_HEIGHT_OFFSET = 72.0f;
}

int main()
{
    Memory m(L"cs2.exe");
    while (!m.attach())
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto c_mod = m.get_module_info(L"client.dll");
    while (c_mod.base == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        c_mod = m.get_module_info(L"client.dll");
    }
    Overlay ov;
    if (ov.initialize() != OverlayStatus::Success)
        return 1;
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    SharedState s;
    s.players_buffer[0].reserve(64);
    s.players_buffer[1].reserve(64);
    std::thread t(cheat::worker, std::ref(m), c_mod.base, std::ref(s));
    Config cfg;
    bool menu = false;
    render::setup_monochrome();
    char cfg_input[64] = "default";
    std::string cfg_name = "default";
    while (ov.handle_messages())
    {
        if (GetAsyncKeyState(VK_INSERT) & 0x0001)
        {
            menu = !menu;
            ov.toggle_click_through(!menu);
        }
        std::vector<PlayerData> src_players;
        ViewMatrix vm;
        std::uint8_t local_team = 0;
        {
            std::lock_guard<std::mutex> lock(s.write_mutex);
            int r_idx = s.read_index.load();
            src_players = s.players_buffer[r_idx];
            vm = s.vm_buffer[r_idx];
            local_team = s.local_team_buffer[r_idx];
        }
        std::vector<PlayerData> render_players;
        render_players.reserve(src_players.size());
        for (const auto &p : src_players)
        {
            if (!cfg.esp_teammates && p.team == local_team)
                continue;
            PlayerData rp = p;
            bool feet_w2s = math::w2s(rp.position, rp.feet_screen, vm, sw, sh);
            bool head_w2s = math::w2s({rp.position.x, rp.position.y, rp.position.z + consts::HEAD_HEIGHT_OFFSET}, rp.head_screen, vm, sw, sh);
            rp.is_on_screen = feet_w2s && head_w2s;
            if (rp.is_on_screen)
                render_players.push_back(std::move(rp));
        }
        ov.begin_frame();
        if (menu)
        {
            render::draw_menu(cfg, menu, cfg_input, cfg_name);
        }
        if (cfg.esp_enabled)
        {
            render::draw_esp(ImGui::GetBackgroundDrawList(), render_players, cfg, sw, sh);
        }
        ov.end_frame();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    s.is_running = false;
    if (t.joinable())
        t.join();
    return 0;
}
