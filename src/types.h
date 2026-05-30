#pragma once
#include <string>
#include <cstdint>
#include <array>
#include <vector>
#include <mutex>
#include <atomic>

struct Vector3
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
    std::uint8_t team = 0;
    Vector3 position;
    Vector3 feet_screen;
    Vector3 head_screen;
    bool is_alive = false;
    bool is_on_screen = false;
};

#pragma pack(push, 1)
struct Config
{
    std::uint32_t magic = 0x564F5201;
    std::uint32_t version = 3;
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
    std::atomic<int> read_index{0};
    std::atomic<int> write_index{1};
    std::mutex write_mutex;
    std::atomic<bool> is_running{true};
};
