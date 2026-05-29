#pragma once
#include "../types.h"
#include "imgui.h"
#include <string>

namespace render
{
    void setup_monochrome();
    void draw_esp(ImDrawList *const dl, const std::vector<PlayerData> &players, const Config &cfg, int sw, int sh);
    void draw_menu(Config &cfg, bool &menu, char *cfg_input, std::string &cfg_name);
}
