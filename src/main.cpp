#include "memory.h"
#include "offsets.h"
#include "overlay.h"
#include "imgui.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <cmath>
#include <numbers>
#include <filesystem>
#include <fstream>
#include <string>
#include <algorithm>
#include <array>

struct Vector3
{
    float x, y, z;
};

struct ViewMatrix
{
    float matrix[4][4];
};

struct PlayerData
{
    std::string name;
    int health;
    std::uint8_t team;
    Vector3 position;
    Vector3 feet_screen;
    Vector3 head_screen;
    bool is_alive;
    bool is_on_screen;
};

struct Config
{
    std::uint32_t magic = 0x564F5201;
    bool esp_enabled = true;
    bool esp_outline = true;
    bool show_boxes = true;
    int box_type = 0;
    float box_border = 1.0f;
    bool show_health = true;
    int hp_bar_pos = 0;
    bool show_names = true;
    int name_pos = 0;
    bool show_snaplines = false;
    int snapline_type = 2;
    float snapline_border = 1.0f;
    float color_t[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float color_ct[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float color_txt[4] = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct SharedState
{
    std::vector<PlayerData> players;
    ViewMatrix view_matrix;
    std::uintptr_t local_pawn;
    int local_index;
    std::mutex mutex;
    std::atomic<bool> is_running{true};
};

namespace consts
{
    constexpr int MAX_PLAYERS = 64;
    constexpr std::uint32_t HANDLE_MASK = 0x7FFF;
    constexpr std::uint32_t INVALID_HANDLE = 0xFFFFFFFF;
    constexpr int ENTITY_LIST_STRIDE = 112;
    constexpr int ENTITY_LIST_PAGE_SHIFT = 9;
    constexpr int ENTITY_LIST_PAGE_MASK = 0x1FF;
    constexpr float W2S_NEAR_PLANE = 0.01f;
    constexpr float HEAD_HEIGHT_OFFSET = 72.0f;
    constexpr std::uint32_t CONFIG_MAGIC = 0x564F5201;
    constexpr std::uint16_t KEY_PRESSED_MASK = 0x8000;
    constexpr std::uint16_t KEY_TOGGLED_MASK = 0x0001;
    constexpr int MAX_NAME_LENGTH = 128;
    constexpr int MAX_CONFIG_NAME_LENGTH = 64;
    constexpr float MAX_HEALTH_PERCENT = 1.0f;
    constexpr float MIN_HEALTH_PERCENT = 0.0f;
    constexpr int HEALTH_DIVISOR = 100;
    constexpr int BYTE_MAX = 255;
    constexpr int BOX_PADDING = 1;
    constexpr int HP_BAR_WIDTH = 3;
    constexpr int HP_BAR_OFFSET = 6;
    constexpr int TEXT_PADDING = 2;
}

namespace config_manager
{
    const std::string path = "C:\\Vor";

    void save(const std::string &name, const Config &cfg)
    {
        std::error_code ec;
        std::filesystem::create_directories(path, ec);
        std::ofstream f(path + "\\" + name + ".cfg", std::ios::binary);
        if (f.is_open())
        {
            f.write(reinterpret_cast<const char *>(&cfg), sizeof(Config));
        }
    }

    void load(const std::string &name, Config &cfg)
    {
        std::ifstream f(path + "\\" + name + ".cfg", std::ios::binary);
        if (f.is_open())
        {
            Config tmp;
            f.read(reinterpret_cast<char *>(&tmp), sizeof(Config));
            if (tmp.magic == consts::CONFIG_MAGIC)
            {
                cfg = tmp;
            }
        }
    }

    void remove(const std::string &name)
    {
        std::error_code ec;
        std::filesystem::remove(path + "\\" + name + ".cfg", ec);
    }

    std::vector<std::string> list()
    {
        std::vector<std::string> res;
        std::error_code ec;
        if (!std::filesystem::exists(path, ec))
            return res;
        for (const auto &e : std::filesystem::directory_iterator(path, ec))
        {
            if (e.path().extension() == ".cfg")
            {
                res.push_back(e.path().stem().string());
            }
        }
        return res;
    }
}

void setup_monochrome()
{
    auto &s = ImGui::GetStyle();
    s.WindowRounding = 0.0f;
    s.FrameRounding = 0.0f;
    s.PopupRounding = 0.0f;
    s.ScrollbarRounding = 0.0f;
    s.GrabRounding = 0.0f;
    s.TabRounding = 0.0f;
    s.WindowBorderSize = 1.0f;
    s.FrameBorderSize = 1.0f;

    auto *c = s.Colors;
    c[ImGuiCol_Text] = {1.0f, 1.0f, 1.0f, 1.0f};
    c[ImGuiCol_WindowBg] = {0.05f, 0.05f, 0.05f, 1.0f};
    c[ImGuiCol_PopupBg] = {0.05f, 0.05f, 0.05f, 1.0f};
    c[ImGuiCol_Border] = {0.25f, 0.25f, 0.25f, 1.0f};
    c[ImGuiCol_FrameBg] = {0.08f, 0.08f, 0.08f, 1.0f};
    c[ImGuiCol_FrameBgHovered] = {0.15f, 0.15f, 0.15f, 1.0f};
    c[ImGuiCol_FrameBgActive] = {0.20f, 0.20f, 0.20f, 1.0f};
    c[ImGuiCol_TitleBg] = {0.08f, 0.08f, 0.08f, 1.0f};
    c[ImGuiCol_TitleBgActive] = {0.08f, 0.08f, 0.08f, 1.0f};
    c[ImGuiCol_Button] = {0.10f, 0.10f, 0.10f, 1.0f};
    c[ImGuiCol_ButtonHovered] = {0.20f, 0.20f, 0.20f, 1.0f};
    c[ImGuiCol_ButtonActive] = {0.30f, 0.30f, 0.30f, 1.0f};
    c[ImGuiCol_Header] = {0.12f, 0.12f, 0.12f, 1.0f};
    c[ImGuiCol_HeaderHovered] = {0.20f, 0.20f, 0.20f, 1.0f};
    c[ImGuiCol_HeaderActive] = {0.30f, 0.30f, 0.30f, 1.0f};
    c[ImGuiCol_Tab] = {0.08f, 0.08f, 0.08f, 1.0f};
    c[ImGuiCol_TabHovered] = {0.20f, 0.20f, 0.20f, 1.0f};
    c[ImGuiCol_TabActive] = {0.25f, 0.25f, 0.25f, 1.0f};
    c[ImGuiCol_CheckMark] = {1.0f, 1.0f, 1.0f, 1.0f};
    c[ImGuiCol_SliderGrab] = {0.40f, 0.40f, 0.40f, 1.0f};
    c[ImGuiCol_SliderGrabActive] = {0.60f, 0.60f, 0.60f, 1.0f};
    c[ImGuiCol_Separator] = {0.20f, 0.20f, 0.20f, 1.0f};
}

bool w2s(const Vector3 &world, Vector3 &screen, const ViewMatrix &vm, int w, int h)
{
    float clip_w = vm.matrix[3][0] * world.x + vm.matrix[3][1] * world.y + vm.matrix[3][2] * world.z + vm.matrix[3][3];
    if (clip_w < consts::W2S_NEAR_PLANE)
        return false;

    float x = vm.matrix[0][0] * world.x + vm.matrix[0][1] * world.y + vm.matrix[0][2] * world.z + vm.matrix[0][3];
    float y = vm.matrix[1][0] * world.x + vm.matrix[1][1] * world.y + vm.matrix[1][2] * world.z + vm.matrix[1][3];

    screen.x = (w / 2.0f) + (x / clip_w * (w / 2.0f));
    screen.y = (h / 2.0f) - (y / clip_w * (h / 2.0f));
    return true;
}

std::uintptr_t get_ent(const Memory &m, std::uintptr_t base, int i)
{
    std::uintptr_t list = m.read<std::uintptr_t>(base + offsets::dwEntityList);
    if (!list)
        return 0;

    std::uintptr_t entry = m.read<std::uintptr_t>(list + (8 * (i >> consts::ENTITY_LIST_PAGE_SHIFT)) + 16);
    if (!entry)
        return 0;

    return m.read<std::uintptr_t>(entry + consts::ENTITY_LIST_STRIDE * (i & consts::ENTITY_LIST_PAGE_MASK));
}

std::uintptr_t get_pawn(const Memory &m, std::uintptr_t base, std::uint32_t handle)
{
    if (handle == 0 || handle == consts::INVALID_HANDLE)
        return 0;

    return get_ent(m, base, static_cast<int>(handle & consts::HANDLE_MASK));
}

void worker(const Memory &m, std::uintptr_t c_base, std::uintptr_t e_base, SharedState &s)
{
    while (s.is_running)
    {
        std::uintptr_t lp = m.read<std::uintptr_t>(c_base + offsets::dwLocalPlayerPawn);
        ViewMatrix vm = m.read<ViewMatrix>(c_base + offsets::dwViewMatrix);

        std::vector<PlayerData> tmp;
        for (int i = 1; i <= consts::MAX_PLAYERS; ++i)
        {
            std::uintptr_t ctrl = get_ent(m, c_base, i);
            if (!ctrl)
                continue;

            std::uintptr_t p = get_pawn(m, c_base, m.read<std::uint32_t>(ctrl + offsets::m_hPlayerPawn));
            if (!p || p == lp)
                continue;

            if (!m.read<bool>(ctrl + offsets::m_bPawnIsAlive))
                continue;

            PlayerData d;
            d.health = m.read<int>(p + offsets::m_iHealth);
            d.team = m.read<std::uint8_t>(p + offsets::m_iTeamNum);
            d.position = m.read<Vector3>(p + offsets::m_vOldOrigin);
            d.is_alive = true;

            char buf[consts::MAX_NAME_LENGTH]{};
            m.read_string(ctrl + offsets::m_iszPlayerName, buf, consts::MAX_NAME_LENGTH);
            d.name = buf;

            tmp.push_back(d);
        }

        {
            std::lock_guard<std::mutex> lock(s.mutex);
            s.players = std::move(tmp);
            s.view_matrix = vm;
            s.local_pawn = lp;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main()
{
    Memory m(L"cs2.exe");
    while (!m.attach())
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto c_mod = m.get_module_info(L"client.dll");
    auto e_mod = m.get_module_info(L"engine2.dll");

    while (c_mod.base == 0 || e_mod.base == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        c_mod = m.get_module_info(L"client.dll");
        e_mod = m.get_module_info(L"engine2.dll");
    }

    Overlay ov;
    ov.initialize();

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    SharedState s;
    std::thread t(worker, std::ref(m), c_mod.base, e_mod.base, std::ref(s));

    Config cfg;
    bool menu = false;
    setup_monochrome();

    char cfg_n[consts::MAX_CONFIG_NAME_LENGTH] = "default";

    while (ov.handle_messages())
    {
        if (GetAsyncKeyState(VK_INSERT) & consts::KEY_TOGGLED_MASK)
        {
            menu = !menu;
            ov.toggle_click_through(!menu);
        }

        std::vector<PlayerData> players;
        ViewMatrix vm;
        {
            std::lock_guard<std::mutex> lock(s.mutex);
            players = s.players;
            vm = s.view_matrix;
        }

        for (auto &p : players)
        {
            bool feet_w2s = w2s(p.position, p.feet_screen, vm, sw, sh);
            bool head_w2s = w2s({p.position.x, p.position.y, p.position.z + consts::HEAD_HEIGHT_OFFSET}, p.head_screen, vm, sw, sh);
            p.is_on_screen = feet_w2s && head_w2s;
        }

        ov.begin_frame();

        if (menu)
        {
            ImGui::Begin("VorExternal", &menu, ImGuiWindowFlags_NoCollapse);
            if (ImGui::BeginTabBar("##Tabs"))
            {
                if (ImGui::BeginTabItem("ESP"))
                {
                    ImGui::Checkbox("Enable", &cfg.esp_enabled);
                    ImGui::Checkbox("Outline", &cfg.esp_outline);
                    ImGui::Checkbox("Boxes", &cfg.show_boxes);
                    const char *bt[] = {"Frame", "Corners"};
                    ImGui::Combo("Boxes Type", &cfg.box_type, bt, 2);
                    ImGui::SliderFloat("Border##Box", &cfg.box_border, 1, 3);
                    ImGui::Checkbox("HP Bar", &cfg.show_health);
                    const char *hp[] = {"Left", "Right", "Top", "Bottom"};
                    ImGui::Combo("Position##HP", &cfg.hp_bar_pos, hp, 4);
                    ImGui::Checkbox("Nicknames", &cfg.show_names);
                    const char *np[] = {"Top", "Bottom", "Left", "Right"};
                    ImGui::Combo("Position##Name", &cfg.name_pos, np, 4);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Colors"))
                {
                    ImGui::ColorEdit4("Terrorist", cfg.color_t);
                    ImGui::ColorEdit4("Counter-Terrorist", cfg.color_ct);
                    ImGui::ColorEdit4("Text", cfg.color_txt);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Config"))
                {
                    ImGui::InputText("Name##Cfg", cfg_n, consts::MAX_CONFIG_NAME_LENGTH);
                    if (ImGui::Button("Save##Cfg"))
                        config_manager::save(cfg_n, cfg);
                    ImGui::SameLine();
                    if (ImGui::Button("Load##Cfg"))
                        config_manager::load(cfg_n, cfg);
                    ImGui::SameLine();
                    if (ImGui::Button("Delete##Cfg"))
                        config_manager::remove(cfg_n);

                    ImGui::Separator();
                    for (const auto &n : config_manager::list())
                    {
                        if (ImGui::Selectable(n.c_str()))
                        {
                            std::strncpy(cfg_n, n.c_str(), consts::MAX_CONFIG_NAME_LENGTH - 1);
                            cfg_n[consts::MAX_CONFIG_NAME_LENGTH - 1] = '\0';
                            config_manager::load(n, cfg);
                        }
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }

        if (cfg.esp_enabled)
        {
            auto *dl = ImGui::GetBackgroundDrawList();
            for (const auto &p : players)
            {
                if (!p.is_on_screen)
                    continue;

                float h = p.feet_screen.y - p.head_screen.y;
                if (h <= 0.0f)
                    continue;

                float w = h / 2.0f;
                int il = static_cast<int>(p.head_screen.x - w / 2.0f);
                int it = static_cast<int>(p.head_screen.y);
                int ir = static_cast<int>(il + w);
                int ib = static_cast<int>(p.feet_screen.y);

                float l = static_cast<float>(il);
                float t = static_cast<float>(it);
                float r = static_cast<float>(ir);
                float b = static_cast<float>(ib);

                ImU32 clr = ImGui::ColorConvertFloat4ToU32(p.team == 2 ? *(ImVec4 *)cfg.color_t : *(ImVec4 *)cfg.color_ct);

                if (cfg.show_boxes)
                {
                    if (cfg.box_type == 0)
                    {
                        if (cfg.esp_outline)
                            dl->AddRect({l - consts::BOX_PADDING, t - consts::BOX_PADDING}, {r + consts::BOX_PADDING, b + consts::BOX_PADDING}, 0xFF000000);
                        dl->AddRect({l, t}, {r, b}, clr, 0, 0, cfg.box_border);
                    }
                    else
                    {
                        auto draw_c = [&](ImU32 c, float th)
                        {
                            float lw = w / 4.0f;
                            float lh = h / 4.0f;
                            dl->AddLine({l, t}, {l + lw, t}, c, th);
                            dl->AddLine({l, t}, {l, t + lh}, c, th);
                            dl->AddLine({r, t}, {r - lw, t}, c, th);
                            dl->AddLine({r, t}, {r, t + lh}, c, th);
                            dl->AddLine({l, b}, {l + lw, b}, c, th);
                            dl->AddLine({l, b}, {l, b - lh}, c, th);
                            dl->AddLine({r, b}, {r - lw, b}, c, th);
                            dl->AddLine({r, b}, {r, b - lh}, c, th);
                        };
                        if (cfg.esp_outline)
                            draw_c(0xFF000000, cfg.box_border + 2.0f);
                        draw_c(clr, cfg.box_border);
                    }
                }

                if (cfg.show_health)
                {
                    float pct = std::clamp(static_cast<float>(p.health) / consts::HEALTH_DIVISOR, consts::MIN_HEALTH_PERCENT, consts::MAX_HEALTH_PERCENT);
                    ImU32 hclr = IM_COL32(static_cast<int>((consts::MAX_HEALTH_PERCENT - pct) * consts::BYTE_MAX), static_cast<int>(pct * consts::BYTE_MAX), 0, consts::BYTE_MAX);

                    float bl, bt, br, bb, fl, ft, fr, fb;
                    if (cfg.hp_bar_pos == 0)
                    {
                        bl = l - consts::HP_BAR_OFFSET;
                        bt = t;
                        br = l - consts::HP_BAR_WIDTH;
                        bb = b;
                        fl = bl;
                        ft = b - (h * pct);
                        fr = br;
                        fb = b;
                    }
                    else if (cfg.hp_bar_pos == 1)
                    {
                        bl = r + consts::HP_BAR_WIDTH;
                        bt = t;
                        br = r + consts::HP_BAR_OFFSET;
                        bb = b;
                        fl = bl;
                        ft = b - (h * pct);
                        fr = br;
                        fb = b;
                    }
                    else if (cfg.hp_bar_pos == 2)
                    {
                        bl = l;
                        bt = t - consts::HP_BAR_OFFSET;
                        br = r;
                        bb = t - consts::HP_BAR_WIDTH;
                        fl = l;
                        ft = bt;
                        fr = l + (w * pct);
                        fb = bb;
                    }
                    else
                    {
                        bl = l;
                        bt = b + consts::HP_BAR_WIDTH;
                        br = r;
                        bb = b + consts::HP_BAR_OFFSET;
                        fl = l;
                        ft = bt;
                        fr = l + (w * pct);
                        fb = bb;
                    }

                    if (cfg.esp_outline)
                        dl->AddRectFilled({bl - consts::BOX_PADDING, bt - consts::BOX_PADDING}, {br + consts::BOX_PADDING, bb + consts::BOX_PADDING}, 0xFF000000);
                    dl->AddRectFilled({bl, bt}, {br, bb}, IM_COL32(40, 40, 40, 200));
                    dl->AddRectFilled({fl, ft}, {fr, fb}, hclr);
                }

                if (cfg.show_names)
                {
                    ImVec2 sz = ImGui::CalcTextSize(p.name.c_str());
                    ImVec2 pos;
                    if (cfg.name_pos == 0)
                        pos = {l + w / 2 - sz.x / 2, t - sz.y - consts::TEXT_PADDING};
                    else if (cfg.name_pos == 1)
                        pos = {l + w / 2 - sz.x / 2, b + consts::TEXT_PADDING};
                    else if (cfg.name_pos == 2)
                        pos = {l - sz.x - consts::TEXT_PADDING, t + h / 2 - sz.y / 2};
                    else
                        pos = {r + consts::TEXT_PADDING, t + h / 2 - sz.y / 2};

                    if (cfg.esp_outline)
                    {
                        dl->AddText({pos.x - consts::BOX_PADDING, pos.y}, 0xFF000000, p.name.c_str());
                        dl->AddText({pos.x + consts::BOX_PADDING, pos.y}, 0xFF000000, p.name.c_str());
                        dl->AddText({pos.x, pos.y - consts::BOX_PADDING}, 0xFF000000, p.name.c_str());
                        dl->AddText({pos.x, pos.y + consts::BOX_PADDING}, 0xFF000000, p.name.c_str());
                    }
                    dl->AddText(pos, ImGui::ColorConvertFloat4ToU32(*(ImVec4 *)cfg.color_txt), p.name.c_str());
                }
            }
        }

        ov.end_frame();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    s.is_running = false;
    if (t.joinable())
        t.join();

    return 0;
}
