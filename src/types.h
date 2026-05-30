#pragma once
#include <string>
#include <cstdint>
#include <array>
#include <vector>
#include <mutex>
#include <atomic>
#include <cmath>

struct Vector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct QAngle
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct ViewMatrix
{
    float matrix[4][4]{};
};

struct PlayerData
{
    std::string name;
    int health = 0;
    int armor = 0;
    int money = 0;
    std::uint8_t team = 0;
    Vector3 position;
    Vector3 velocity;
    QAngle eye_angles;
    Vector3 feet_screen;
    Vector3 head_screen;
    float distance = 0.0f;
    bool is_alive = false;
    bool is_on_screen = false;
    bool has_defuser = false;
    bool is_scoped = false;
    bool is_crouching = false;
    bool is_in_air = false;
    float flash_duration = 0.0f;
};

struct PlantedC4Data
{
    bool is_planted = false;
    Vector3 position;
    Vector3 screen_pos;
    bool is_on_screen = false;
    float distance = 0.0f;
    bool being_defused = false;
};

#pragma pack(push, 1)
struct Config
{
    std::uint32_t magic = 0x564F5201;
    std::uint32_t version = 4;
    bool esp_enabled = true;
    bool esp_teammates = false;
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
    bool show_armor = true;
    bool show_money = false;
    bool show_distance = true;
    bool show_defuse_kit = true;
    bool show_scoped = true;
    bool show_flash = true;
    bool show_view_dir = false;
    bool show_offscreen_arrows = false;
    bool esp_c4 = true;
    float c4_color[4] = {1.0f, 0.5f, 0.0f, 1.0f};
    float color_t[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    float color_ct[4] = {0.0f, 0.5f, 1.0f, 1.0f};
    float color_txt[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    int language = 0;
};
#pragma pack(pop)

enum class ClickThroughMode
{
    Interactive,
    Transparent
};

struct SharedState
{
    std::array<std::vector<PlayerData>, 2> players_buffer;
    std::array<ViewMatrix, 2> vm_buffer;
    std::array<std::uint8_t, 2> local_team_buffer;
    std::array<Vector3, 2> local_pos_buffer;
    std::array<PlantedC4Data, 2> c4_buffer;
    std::atomic<int> read_index{0};
    std::atomic<int> write_index{1};
    std::mutex write_mutex;
    std::atomic<bool> is_running{true};
};
