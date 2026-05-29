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

        int r_idx = s.read_index.load();
        std::vector<PlayerData> render_players = s.players_buffer[r_idx];
        ViewMatrix vm = s.vm_buffer[r_idx];

        for (auto &p : render_players)
        {
            bool feet_w2s = math::w2s(p.position, p.feet_screen, vm, sw, sh);
            bool head_w2s = math::w2s({p.position.x, p.position.y, p.position.z + 72.0f}, p.head_screen, vm, sw, sh);
            p.is_on_screen = feet_w2s && head_w2s;
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
