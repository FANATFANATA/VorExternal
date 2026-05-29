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
#include <optional>

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

#pragma pack(push, 1)
struct Config
{
    std::uint32_t magic = 0x564F5201;
    std::uint32_t version = 1;
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
    int snapline_type = 0;
    float snapline_border = 1.0f;
    float color_t[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    float color_ct[4] = {0.0f, 0.5f, 1.0f, 1.0f};
    float color_txt[4] = {1.0f, 1.0f, 1.0f, 1.0f};
};
#pragma pack(pop)

struct SharedState
{
    std::array<std::vector<PlayerData>, 2> players_buffer;
    std::array<ViewMatrix, 2> vm_buffer;
    std::array<std::uintptr_t, 2> lp_buffer;
    std::atomic<int> read_index{0};
    std::atomic<int> write_index{1};
    std::mutex write_mutex;
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
    constexpr std::uint8_t TEAM_TERRORIST = 2;

    constexpr float W2S_HALF = 2.0f;
    constexpr float BOX_WIDTH_DIVISOR = 2.0f;
    constexpr float CORNER_DIVISOR = 4.0f;

    constexpr float UI_WINDOW_ROUNDING = 4.0f;
    constexpr float UI_FRAME_ROUNDING = 2.0f;
    constexpr float UI_POPUP_ROUNDING = 4.0f;
    constexpr float UI_SCROLLBAR_ROUNDING = 4.0f;
    constexpr float UI_GRAB_ROUNDING = 2.0f;
    constexpr float UI_TAB_ROUNDING = 2.0f;
    constexpr float UI_WINDOW_BORDER = 1.0f;
    constexpr float UI_FRAME_BORDER = 0.0f;
    constexpr float UI_WINDOW_PAD_X = 10.0f;
    constexpr float UI_WINDOW_PAD_Y = 10.0f;
    constexpr float UI_FRAME_PAD_X = 8.0f;
    constexpr float UI_FRAME_PAD_Y = 4.0f;
    constexpr float UI_ITEM_SPACE_X = 8.0f;
    constexpr float UI_ITEM_SPACE_Y = 6.0f;

    constexpr float UI_COL_TEXT_R = 0.9f;
    constexpr float UI_COL_TEXT_G = 0.9f;
    constexpr float UI_COL_TEXT_B = 0.9f;
    constexpr float UI_COL_TEXT_A = 1.0f;

    constexpr float UI_COL_WIN_BG_R = 0.08f;
    constexpr float UI_COL_WIN_BG_G = 0.08f;
    constexpr float UI_COL_WIN_BG_B = 0.08f;
    constexpr float UI_COL_WIN_BG_A = 0.95f;

    constexpr float UI_COL_POPUP_BG_R = 0.1f;
    constexpr float UI_COL_POPUP_BG_G = 0.1f;
    constexpr float UI_COL_POPUP_BG_B = 0.1f;
    constexpr float UI_COL_POPUP_BG_A = 0.95f;

    constexpr float UI_COL_BORDER_R = 0.2f;
    constexpr float UI_COL_BORDER_G = 0.2f;
    constexpr float UI_COL_BORDER_B = 0.2f;
    constexpr float UI_COL_BORDER_A = 1.0f;

    constexpr float UI_COL_FRAME_R = 0.15f;
    constexpr float UI_COL_FRAME_G = 0.15f;
    constexpr float UI_COL_FRAME_B = 0.15f;
    constexpr float UI_COL_FRAME_A = 1.0f;

    constexpr float UI_COL_FRAME_HOV_R = 0.2f;
    constexpr float UI_COL_FRAME_HOV_G = 0.2f;
    constexpr float UI_COL_FRAME_HOV_B = 0.2f;
    constexpr float UI_COL_FRAME_HOV_A = 1.0f;

    constexpr float UI_COL_FRAME_ACT_R = 0.25f;
    constexpr float UI_COL_FRAME_ACT_G = 0.25f;
    constexpr float UI_COL_FRAME_ACT_B = 0.25f;
    constexpr float UI_COL_FRAME_ACT_A = 1.0f;

    constexpr float UI_COL_TITLE_BG_R = 0.05f;
    constexpr float UI_COL_TITLE_BG_G = 0.05f;
    constexpr float UI_COL_TITLE_BG_B = 0.05f;
    constexpr float UI_COL_TITLE_BG_A = 1.0f;

    constexpr float UI_COL_TITLE_ACT_R = 0.1f;
    constexpr float UI_COL_TITLE_ACT_G = 0.1f;
    constexpr float UI_COL_TITLE_ACT_B = 0.1f;
    constexpr float UI_COL_TITLE_ACT_A = 1.0f;

    constexpr float UI_COL_BTN_R = 0.2f;
    constexpr float UI_COL_BTN_G = 0.2f;
    constexpr float UI_COL_BTN_B = 0.2f;
    constexpr float UI_COL_BTN_A = 1.0f;

    constexpr float UI_COL_BTN_HOV_R = 0.3f;
    constexpr float UI_COL_BTN_HOV_G = 0.3f;
    constexpr float UI_COL_BTN_HOV_B = 0.3f;
    constexpr float UI_COL_BTN_HOV_A = 1.0f;

    constexpr float UI_COL_BTN_ACT_R = 0.4f;
    constexpr float UI_COL_BTN_ACT_G = 0.4f;
    constexpr float UI_COL_BTN_ACT_B = 0.4f;
    constexpr float UI_COL_BTN_ACT_A = 1.0f;

    constexpr float UI_COL_HDR_R = 0.25f;
    constexpr float UI_COL_HDR_G = 0.25f;
    constexpr float UI_COL_HDR_B = 0.25f;
    constexpr float UI_COL_HDR_A = 1.0f;

    constexpr float UI_COL_HDR_HOV_R = 0.35f;
    constexpr float UI_COL_HDR_HOV_G = 0.35f;
    constexpr float UI_COL_HDR_HOV_B = 0.35f;
    constexpr float UI_COL_HDR_HOV_A = 1.0f;

    constexpr float UI_COL_HDR_ACT_R = 0.45f;
    constexpr float UI_COL_HDR_ACT_G = 0.45f;
    constexpr float UI_COL_HDR_ACT_B = 0.45f;
    constexpr float UI_COL_HDR_ACT_A = 1.0f;

    constexpr float UI_COL_TAB_R = 0.15f;
    constexpr float UI_COL_TAB_G = 0.15f;
    constexpr float UI_COL_TAB_B = 0.15f;
    constexpr float UI_COL_TAB_A = 1.0f;

    constexpr float UI_COL_TAB_HOV_R = 0.3f;
    constexpr float UI_COL_TAB_HOV_G = 0.3f;
    constexpr float UI_COL_TAB_HOV_B = 0.3f;
    constexpr float UI_COL_TAB_HOV_A = 1.0f;

    constexpr float UI_COL_TAB_ACT_R = 0.25f;
    constexpr float UI_COL_TAB_ACT_G = 0.25f;
    constexpr float UI_COL_TAB_ACT_B = 0.25f;
    constexpr float UI_COL_TAB_ACT_A = 1.0f;

    constexpr float UI_COL_CHECK_R = 0.9f;
    constexpr float UI_COL_CHECK_G = 0.9f;
    constexpr float UI_COL_CHECK_B = 0.9f;
    constexpr float UI_COL_CHECK_A = 1.0f;

    constexpr float UI_COL_SLIDER_R = 0.6f;
    constexpr float UI_COL_SLIDER_G = 0.6f;
    constexpr float UI_COL_SLIDER_B = 0.6f;
    constexpr float UI_COL_SLIDER_A = 1.0f;

    constexpr float UI_COL_SLIDER_ACT_R = 0.8f;
    constexpr float UI_COL_SLIDER_ACT_G = 0.8f;
    constexpr float UI_COL_SLIDER_ACT_B = 0.8f;
    constexpr float UI_COL_SLIDER_ACT_A = 1.0f;

    constexpr float UI_COL_SEP_R = 0.2f;
    constexpr float UI_COL_SEP_G = 0.2f;
    constexpr float UI_COL_SEP_B = 0.2f;
    constexpr float UI_COL_SEP_A = 1.0f;

    constexpr float UI_DEFAULT_WINDOW_W = 450.0f;
    constexpr float UI_DEFAULT_WINDOW_H = 350.0f;

    constexpr float SNAPLINE_OUTLINE_ADD = 2.0f;
    constexpr float BOX_OUTLINE_ADD = 2.0f;

    constexpr int HP_BAR_BG_R = 40;
    constexpr int HP_BAR_BG_G = 40;
    constexpr int HP_BAR_BG_B = 40;
    constexpr int HP_BAR_BG_A = 200;

    constexpr float SLIDER_MIN = 1.0f;
    constexpr float SLIDER_MAX = 3.0f;
}

namespace config_manager
{
    std::filesystem::path get_dir()
    {
        char *appdata = nullptr;
        std::size_t len = 0;
        if (_dupenv_s(&appdata, &len, "APPDATA") == 0 && appdata)
        {
            std::filesystem::path p = std::filesystem::path(appdata) / "VorExternal";
            free(appdata);
            return p;
        }
        return std::filesystem::current_path() / "VorExternal";
    }

    void save(const std::string &name, const Config &cfg)
    {
        std::error_code ec;
        std::filesystem::path dir = get_dir();
        static_cast<void>(std::filesystem::create_directories(dir, ec));
        std::ofstream f(dir / (name + ".cfg"), std::ios::binary);
        if (f.is_open())
        {
            f.write(reinterpret_cast<const char *>(&cfg), sizeof(Config));
        }
    }

    void load(const std::string &name, Config &cfg)
    {
        std::ifstream f(get_dir() / (name + ".cfg"), std::ios::binary);
        if (f.is_open())
        {
            Config tmp;
            f.read(reinterpret_cast<char *>(&tmp), sizeof(Config));
            if (tmp.magic == consts::CONFIG_MAGIC && tmp.version == cfg.version)
            {
                cfg = tmp;
            }
        }
    }

    void remove(const std::string &name)
    {
        std::error_code ec;
        static_cast<void>(std::filesystem::remove(get_dir() / (name + ".cfg"), ec));
    }

    std::vector<std::string> list()
    {
        std::vector<std::string> res;
        std::error_code ec;
        std::filesystem::path dir = get_dir();
        if (!std::filesystem::exists(dir, ec))
            return res;
        for (const auto &e : std::filesystem::directory_iterator(dir, ec))
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
    s.WindowRounding = consts::UI_WINDOW_ROUNDING;
    s.FrameRounding = consts::UI_FRAME_ROUNDING;
    s.PopupRounding = consts::UI_POPUP_ROUNDING;
    s.ScrollbarRounding = consts::UI_SCROLLBAR_ROUNDING;
    s.GrabRounding = consts::UI_GRAB_ROUNDING;
    s.TabRounding = consts::UI_TAB_ROUNDING;
    s.WindowBorderSize = consts::UI_WINDOW_BORDER;
    s.FrameBorderSize = consts::UI_FRAME_BORDER;
    s.WindowPadding = ImVec2(consts::UI_WINDOW_PAD_X, consts::UI_WINDOW_PAD_Y);
    s.FramePadding = ImVec2(consts::UI_FRAME_PAD_X, consts::UI_FRAME_PAD_Y);
    s.ItemSpacing = ImVec2(consts::UI_ITEM_SPACE_X, consts::UI_ITEM_SPACE_Y);

    auto *c = s.Colors;
    c[ImGuiCol_Text] = ImVec4(consts::UI_COL_TEXT_R, consts::UI_COL_TEXT_G, consts::UI_COL_TEXT_B, consts::UI_COL_TEXT_A);
    c[ImGuiCol_WindowBg] = ImVec4(consts::UI_COL_WIN_BG_R, consts::UI_COL_WIN_BG_G, consts::UI_COL_WIN_BG_B, consts::UI_COL_WIN_BG_A);
    c[ImGuiCol_PopupBg] = ImVec4(consts::UI_COL_POPUP_BG_R, consts::UI_COL_POPUP_BG_G, consts::UI_COL_POPUP_BG_B, consts::UI_COL_POPUP_BG_A);
    c[ImGuiCol_Border] = ImVec4(consts::UI_COL_BORDER_R, consts::UI_COL_BORDER_G, consts::UI_COL_BORDER_B, consts::UI_COL_BORDER_A);
    c[ImGuiCol_FrameBg] = ImVec4(consts::UI_COL_FRAME_R, consts::UI_COL_FRAME_G, consts::UI_COL_FRAME_B, consts::UI_COL_FRAME_A);
    c[ImGuiCol_FrameBgHovered] = ImVec4(consts::UI_COL_FRAME_HOV_R, consts::UI_COL_FRAME_HOV_G, consts::UI_COL_FRAME_HOV_B, consts::UI_COL_FRAME_HOV_A);
    c[ImGuiCol_FrameBgActive] = ImVec4(consts::UI_COL_FRAME_ACT_R, consts::UI_COL_FRAME_ACT_G, consts::UI_COL_FRAME_ACT_B, consts::UI_COL_FRAME_ACT_A);
    c[ImGuiCol_TitleBg] = ImVec4(consts::UI_COL_TITLE_BG_R, consts::UI_COL_TITLE_BG_G, consts::UI_COL_TITLE_BG_B, consts::UI_COL_TITLE_BG_A);
    c[ImGuiCol_TitleBgActive] = ImVec4(consts::UI_COL_TITLE_ACT_R, consts::UI_COL_TITLE_ACT_G, consts::UI_COL_TITLE_ACT_B, consts::UI_COL_TITLE_ACT_A);
    c[ImGuiCol_Button] = ImVec4(consts::UI_COL_BTN_R, consts::UI_COL_BTN_G, consts::UI_COL_BTN_B, consts::UI_COL_BTN_A);
    c[ImGuiCol_ButtonHovered] = ImVec4(consts::UI_COL_BTN_HOV_R, consts::UI_COL_BTN_HOV_G, consts::UI_COL_BTN_HOV_B, consts::UI_COL_BTN_HOV_A);
    c[ImGuiCol_ButtonActive] = ImVec4(consts::UI_COL_BTN_ACT_R, consts::UI_COL_BTN_ACT_G, consts::UI_COL_BTN_ACT_B, consts::UI_COL_BTN_ACT_A);
    c[ImGuiCol_Header] = ImVec4(consts::UI_COL_HDR_R, consts::UI_COL_HDR_G, consts::UI_COL_HDR_B, consts::UI_COL_HDR_A);
    c[ImGuiCol_HeaderHovered] = ImVec4(consts::UI_COL_HDR_HOV_R, consts::UI_COL_HDR_HOV_G, consts::UI_COL_HDR_HOV_B, consts::UI_COL_HDR_HOV_A);
    c[ImGuiCol_HeaderActive] = ImVec4(consts::UI_COL_HDR_ACT_R, consts::UI_COL_HDR_ACT_G, consts::UI_COL_HDR_ACT_B, consts::UI_COL_HDR_ACT_A);
    c[ImGuiCol_Tab] = ImVec4(consts::UI_COL_TAB_R, consts::UI_COL_TAB_G, consts::UI_COL_TAB_B, consts::UI_COL_TAB_A);
    c[ImGuiCol_TabHovered] = ImVec4(consts::UI_COL_TAB_HOV_R, consts::UI_COL_TAB_HOV_G, consts::UI_COL_TAB_HOV_B, consts::UI_COL_TAB_HOV_A);
    c[ImGuiCol_TabActive] = ImVec4(consts::UI_COL_TAB_ACT_R, consts::UI_COL_TAB_ACT_G, consts::UI_COL_TAB_ACT_B, consts::UI_COL_TAB_ACT_A);
    c[ImGuiCol_CheckMark] = ImVec4(consts::UI_COL_CHECK_R, consts::UI_COL_CHECK_G, consts::UI_COL_CHECK_B, consts::UI_COL_CHECK_A);
    c[ImGuiCol_SliderGrab] = ImVec4(consts::UI_COL_SLIDER_R, consts::UI_COL_SLIDER_G, consts::UI_COL_SLIDER_B, consts::UI_COL_SLIDER_A);
    c[ImGuiCol_SliderGrabActive] = ImVec4(consts::UI_COL_SLIDER_ACT_R, consts::UI_COL_SLIDER_ACT_G, consts::UI_COL_SLIDER_ACT_B, consts::UI_COL_SLIDER_ACT_A);
    c[ImGuiCol_Separator] = ImVec4(consts::UI_COL_SEP_R, consts::UI_COL_SEP_G, consts::UI_COL_SEP_B, consts::UI_COL_SEP_A);
}

bool w2s(const Vector3 &world, Vector3 &screen, const ViewMatrix &vm, int w, int h)
{
    float clip_w = vm.matrix[3][0] * world.x + vm.matrix[3][1] * world.y + vm.matrix[3][2] * world.z + vm.matrix[3][3];
    if (clip_w < consts::W2S_NEAR_PLANE)
        return false;

    float x = vm.matrix[0][0] * world.x + vm.matrix[0][1] * world.y + vm.matrix[0][2] * world.z + vm.matrix[0][3];
    float y = vm.matrix[1][0] * world.x + vm.matrix[1][1] * world.y + vm.matrix[1][2] * world.z + vm.matrix[1][3];

    screen.x = (w / consts::W2S_HALF) + (x / clip_w * (w / consts::W2S_HALF));
    screen.y = (h / consts::W2S_HALF) - (y / clip_w * (h / consts::W2S_HALF));
    return true;
}

std::uintptr_t get_ent(const Memory &m, std::uintptr_t base, int i)
{
    auto list_opt = m.read<std::uintptr_t>(base + offsets::dwEntityList);
    if (!list_opt)
        return 0;

    auto entry_opt = m.read<std::uintptr_t>(*list_opt + (8 * (i >> consts::ENTITY_LIST_PAGE_SHIFT)) + 16);
    if (!entry_opt)
        return 0;

    auto ent_opt = m.read<std::uintptr_t>(*entry_opt + consts::ENTITY_LIST_STRIDE * (i & consts::ENTITY_LIST_PAGE_MASK));
    return ent_opt.value_or(0);
}

std::uintptr_t get_pawn(const Memory &m, std::uintptr_t base, std::uint32_t handle)
{
    if (handle == 0 || handle == consts::INVALID_HANDLE)
        return 0;

    return get_ent(m, base, static_cast<int>(handle & consts::HANDLE_MASK));
}

void worker(const Memory &m, std::uintptr_t c_base, std::uintptr_t e_base, SharedState &s)
{
    std::vector<PlayerData> tmp;
    tmp.reserve(consts::MAX_PLAYERS);

    while (s.is_running)
    {
        auto lp_opt = m.read<std::uintptr_t>(c_base + offsets::dwLocalPlayerPawn);
        auto vm_opt = m.read<ViewMatrix>(c_base + offsets::dwViewMatrix);

        if (!lp_opt || !vm_opt)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        std::uintptr_t lp = *lp_opt;
        ViewMatrix vm = *vm_opt;

        tmp.clear();
        for (int i = 1; i <= consts::MAX_PLAYERS; ++i)
        {
            std::uintptr_t ctrl = get_ent(m, c_base, i);
            if (!ctrl)
                continue;

            auto handle_opt = m.read<std::uint32_t>(ctrl + offsets::m_hPlayerPawn);
            if (!handle_opt)
                continue;

            std::uintptr_t p = get_pawn(m, c_base, *handle_opt);
            if (!p || p == lp)
                continue;

            auto alive_opt = m.read<bool>(ctrl + offsets::m_bPawnIsAlive);
            if (!alive_opt || !*alive_opt)
                continue;

            PlayerData d;
            auto health_opt = m.read<int>(p + offsets::m_iHealth);
            auto team_opt = m.read<std::uint8_t>(p + offsets::m_iTeamNum);
            auto pos_opt = m.read<Vector3>(p + offsets::m_vOldOrigin);

            if (!health_opt || !team_opt || !pos_opt)
                continue;

            d.health = *health_opt;
            d.team = *team_opt;
            d.position = *pos_opt;
            d.is_alive = true;

            char buf[consts::MAX_NAME_LENGTH]{};
            if (m.read_string(ctrl + offsets::m_iszPlayerName, buf, consts::MAX_NAME_LENGTH))
            {
                d.name = buf;
            }

            tmp.push_back(std::move(d));
        }

        {
            std::lock_guard<std::mutex> lock(s.write_mutex);
            int w_idx = s.write_index.load();
            s.players_buffer[w_idx] = std::move(tmp);
            s.vm_buffer[w_idx] = vm;
            s.lp_buffer[w_idx] = lp;
            s.read_index.store(w_idx);
            s.write_index.store(w_idx == 0 ? 1 : 0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void draw_esp(ImDrawList *const dl, const std::vector<PlayerData> &players, const Config &cfg, int sw, int sh)
{
    ImVec4 col_t(cfg.color_t[0], cfg.color_t[1], cfg.color_t[2], cfg.color_t[3]);
    ImVec4 col_ct(cfg.color_ct[0], cfg.color_ct[1], cfg.color_ct[2], cfg.color_ct[3]);
    ImVec4 col_txt(cfg.color_txt[0], cfg.color_txt[1], cfg.color_txt[2], cfg.color_txt[3]);

    for (const auto &p : players)
    {
        if (!p.is_on_screen)
            continue;

        float h = p.feet_screen.y - p.head_screen.y;
        if (h <= 0.0f)
            continue;

        float w = h / consts::BOX_WIDTH_DIVISOR;
        float l = p.head_screen.x - w / consts::BOX_WIDTH_DIVISOR;
        float t = p.head_screen.y;
        float r = l + w;
        float b = p.feet_screen.y;

        ImU32 clr = ImGui::ColorConvertFloat4ToU32(p.team == consts::TEAM_TERRORIST ? col_t : col_ct);

        if (cfg.show_snaplines)
        {
            ImVec2 snap_start;
            if (cfg.snapline_type == 0)
                snap_start = {sw / consts::W2S_HALF, static_cast<float>(sh)};
            else if (cfg.snapline_type == 1)
                snap_start = {sw / consts::W2S_HALF, sh / consts::W2S_HALF};
            else
                snap_start = {sw / consts::W2S_HALF, 0.0f};

            ImVec2 snap_end(p.feet_screen.x, p.feet_screen.y);
            if (cfg.esp_outline)
                dl->AddLine(snap_start, snap_end, 0xFF000000, cfg.snapline_border + consts::SNAPLINE_OUTLINE_ADD);
            dl->AddLine(snap_start, snap_end, clr, cfg.snapline_border);
        }

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
                    float lw = w / consts::CORNER_DIVISOR;
                    float lh = h / consts::CORNER_DIVISOR;
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
                    draw_c(0xFF000000, cfg.box_border + consts::BOX_OUTLINE_ADD);
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
            dl->AddRectFilled({bl, bt}, {br, bb}, IM_COL32(consts::HP_BAR_BG_R, consts::HP_BAR_BG_G, consts::HP_BAR_BG_B, consts::HP_BAR_BG_A));
            dl->AddRectFilled({fl, ft}, {fr, fb}, hclr);
        }

        if (cfg.show_names)
        {
            ImVec2 sz = ImGui::CalcTextSize(p.name.c_str());
            ImVec2 pos;
            if (cfg.name_pos == 0)
                pos = {l + w / consts::BOX_WIDTH_DIVISOR - sz.x / consts::BOX_WIDTH_DIVISOR, t - sz.y - consts::TEXT_PADDING};
            else if (cfg.name_pos == 1)
                pos = {l + w / consts::BOX_WIDTH_DIVISOR - sz.x / consts::BOX_WIDTH_DIVISOR, b + consts::TEXT_PADDING};
            else if (cfg.name_pos == 2)
                pos = {l - sz.x - consts::TEXT_PADDING, t + h / consts::BOX_WIDTH_DIVISOR - sz.y / consts::BOX_WIDTH_DIVISOR};
            else
                pos = {r + consts::TEXT_PADDING, t + h / consts::BOX_WIDTH_DIVISOR - sz.y / consts::BOX_WIDTH_DIVISOR};

            if (cfg.esp_outline)
            {
                dl->AddText({pos.x - consts::BOX_PADDING, pos.y}, 0xFF000000, p.name.c_str());
                dl->AddText({pos.x + consts::BOX_PADDING, pos.y}, 0xFF000000, p.name.c_str());
                dl->AddText({pos.x, pos.y - consts::BOX_PADDING}, 0xFF000000, p.name.c_str());
                dl->AddText({pos.x, pos.y + consts::BOX_PADDING}, 0xFF000000, p.name.c_str());
            }
            dl->AddText(pos, ImGui::ColorConvertFloat4ToU32(col_txt), p.name.c_str());
        }
    }
}

void draw_menu(Config &cfg, bool &menu, char *cfg_input, std::string &cfg_name)
{
    ImGui::SetNextWindowSize(ImVec2(consts::UI_DEFAULT_WINDOW_W, consts::UI_DEFAULT_WINDOW_H), ImGuiCond_FirstUseEver);
    ImGui::Begin("VorExternal", &menu, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTabBar("##Tabs"))
    {
        if (ImGui::BeginTabItem("ESP"))
        {
            ImGui::Checkbox("Enable", &cfg.esp_enabled);
            ImGui::Checkbox("Outline", &cfg.esp_outline);
            ImGui::Spacing();

            ImGui::Checkbox("Boxes", &cfg.show_boxes);
            if (cfg.show_boxes)
            {
                ImGui::Indent();
                const char *bt[] = {"Frame", "Corners"};
                ImGui::Combo("Type##Box", &cfg.box_type, bt, 2);
                ImGui::SliderFloat("Border##Box", &cfg.box_border, consts::SLIDER_MIN, consts::SLIDER_MAX, "%.1f");
                ImGui::Unindent();
            }

            ImGui::Spacing();
            ImGui::Checkbox("HP Bar", &cfg.show_health);
            if (cfg.show_health)
            {
                ImGui::Indent();
                const char *hp[] = {"Left", "Right", "Top", "Bottom"};
                ImGui::Combo("Position##HP", &cfg.hp_bar_pos, hp, 4);
                ImGui::Unindent();
            }

            ImGui::Spacing();
            ImGui::Checkbox("Nicknames", &cfg.show_names);
            if (cfg.show_names)
            {
                ImGui::Indent();
                const char *np[] = {"Top", "Bottom", "Left", "Right"};
                ImGui::Combo("Position##Name", &cfg.name_pos, np, 4);
                ImGui::Unindent();
            }

            ImGui::Spacing();
            ImGui::Checkbox("Snaplines", &cfg.show_snaplines);
            if (cfg.show_snaplines)
            {
                ImGui::Indent();
                const char *st[] = {"Bottom", "Center", "Top"};
                ImGui::Combo("Origin##Snap", &cfg.snapline_type, st, 3);
                ImGui::SliderFloat("Border##Snap", &cfg.snapline_border, consts::SLIDER_MIN, consts::SLIDER_MAX, "%.1f");
                ImGui::Unindent();
            }
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
            if (ImGui::InputText("Name##Cfg", cfg_input, consts::MAX_CONFIG_NAME_LENGTH))
            {
                cfg_name = cfg_input;
            }

            float btn_w = ImGui::GetContentRegionAvail().x / 3.0f - 4.0f;
            if (ImGui::Button("Save##Cfg", ImVec2(btn_w, 0)))
                config_manager::save(cfg_name, cfg);
            ImGui::SameLine();
            if (ImGui::Button("Load##Cfg", ImVec2(btn_w, 0)))
                config_manager::load(cfg_name, cfg);
            ImGui::SameLine();
            if (ImGui::Button("Delete##Cfg", ImVec2(btn_w, 0)))
                config_manager::remove(cfg_name);

            ImGui::Separator();
            ImGui::Text("Saved Configs:");
            ImGui::BeginChild("##ConfigList", ImVec2(0, 0), true);
            for (const auto &n : config_manager::list())
            {
                if (ImGui::Selectable(n.c_str()))
                {
                    cfg_name = n;
                    if (cfg_name.length() < consts::MAX_CONFIG_NAME_LENGTH)
                    {
                        std::copy(cfg_name.begin(), cfg_name.end(), cfg_input);
                        cfg_input[cfg_name.length()] = '\0';
                    }
                    config_manager::load(cfg_name, cfg);
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
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
    s.players_buffer[0].reserve(consts::MAX_PLAYERS);
    s.players_buffer[1].reserve(consts::MAX_PLAYERS);
    std::thread t(worker, std::ref(m), c_mod.base, e_mod.base, std::ref(s));

    Config cfg;
    bool menu = false;
    setup_monochrome();

    char cfg_input[consts::MAX_CONFIG_NAME_LENGTH] = "default";
    std::string cfg_name = "default";

    while (ov.handle_messages())
    {
        if (GetAsyncKeyState(VK_INSERT) & consts::KEY_TOGGLED_MASK)
        {
            menu = !menu;
            ov.toggle_click_through(!menu);
        }

        int r_idx = s.read_index.load();
        const auto &players = s.players_buffer[r_idx];
        const auto &vm = s.vm_buffer[r_idx];

        std::vector<PlayerData> render_players = players;
        for (auto &p : render_players)
        {
            bool feet_w2s = w2s(p.position, p.feet_screen, vm, sw, sh);
            bool head_w2s = w2s({p.position.x, p.position.y, p.position.z + consts::HEAD_HEIGHT_OFFSET}, p.head_screen, vm, sw, sh);
            p.is_on_screen = feet_w2s && head_w2s;
        }

        ov.begin_frame();

        if (menu)
        {
            draw_menu(cfg, menu, cfg_input, cfg_name);
        }

        if (cfg.esp_enabled)
        {
            draw_esp(ImGui::GetBackgroundDrawList(), render_players, cfg, sw, sh);
        }

        ov.end_frame();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    s.is_running = false;
    if (t.joinable())
        t.join();

    return 0;
}
