#include "types.h"
#include "math/math.h"
#include "config/config.h"
#include "cheat/cheat.h"
#include "render/render.h"
#include "memory/memory.h"
#include "overlay/overlay.h"
#include <thread>
#include <chrono>
#include <windows.h>

namespace consts
{
    constexpr float HEAD_HEIGHT_OFFSET = 72.0f;
    constexpr int ATTACH_RETRY_DELAY_MS = 500;
    constexpr int FRAME_DELAY_MS = 1;
    constexpr int MAX_PLAYERS_RESERVE = 64;
}

int main()
{
    Memory m(L"cs2.exe");
    while (!m.attach())
        std::this_thread::sleep_for(std::chrono::milliseconds(consts::ATTACH_RETRY_DELAY_MS));

    auto c_mod = m.get_module_info(L"client.dll");
    while (c_mod.base == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(consts::ATTACH_RETRY_DELAY_MS));
        c_mod = m.get_module_info(L"client.dll");
    }

    Overlay ov;
    if (ov.initialize() != OverlayStatus::Success)
        return 1;

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    SharedState s;
    s.players_buffer[0].reserve(consts::MAX_PLAYERS_RESERVE);
    s.players_buffer[1].reserve(consts::MAX_PLAYERS_RESERVE);

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
            ov.toggle_click_through(menu ? ClickThroughMode::Interactive : ClickThroughMode::Transparent);
        }

        std::vector<PlayerData> src_players;
        ViewMatrix vm;
        std::uint8_t local_team = 0;
        PlantedC4Data c4_data;

        int r_idx = s.read_index.load(std::memory_order_acquire);
        src_players = s.players_buffer[r_idx];
        vm = s.vm_buffer[r_idx];
        local_team = s.local_team_buffer[r_idx];
        c4_data = s.c4_buffer[r_idx];

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

            render_players.push_back(std::move(rp));
        }

        if (c4_data.is_planted)
        {
            c4_data.is_on_screen = math::w2s(c4_data.position, c4_data.screen_pos, vm, sw, sh);
        }

        ov.begin_frame();
        render::draw_watermark(ImGui::GetBackgroundDrawList());

        if (menu)
        {
            render::draw_menu(cfg, menu, cfg_input, cfg_name);
        }

        if (cfg.esp_enabled)
        {
            render::draw_esp(ImGui::GetBackgroundDrawList(), render_players, cfg, vm, sw, sh);
        }

        if (cfg.esp_c4 && c4_data.is_planted)
        {
            render::draw_c4(ImGui::GetBackgroundDrawList(), c4_data, cfg, sw, sh);
        }

        ov.end_frame();
        std::this_thread::sleep_for(std::chrono::milliseconds(consts::FRAME_DELAY_MS));
    }

    s.is_running = false;
    if (t.joinable())
        t.join();

    return 0;
}
