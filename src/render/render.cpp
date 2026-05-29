#include "render.h"
#include "../config/config.h"
#include <algorithm>
#include <cstring>

namespace consts
{
    constexpr std::uint8_t TEAM_TERRORIST = 2;
    constexpr float BOX_WIDTH_DIVISOR = 2.0f;
    constexpr float CORNER_DIVISOR = 4.0f;
    constexpr int BOX_PADDING = 1;
    constexpr int HP_BAR_WIDTH = 2;
    constexpr int HP_BAR_OFFSET = 4;
    constexpr int TEXT_PADDING = 2;
    constexpr float MAX_HEALTH_PERCENT = 1.0f;
    constexpr float MIN_HEALTH_PERCENT = 0.0f;
    constexpr int HEALTH_DIVISOR = 100;
    constexpr int BYTE_MAX = 255;
    constexpr float SLIDER_MIN = 1.0f;
    constexpr float SLIDER_MAX = 3.0f;
    constexpr int MAX_CONFIG_NAME_LENGTH = 64;

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

    constexpr ImVec4 COL_TEXT{0.9f, 0.9f, 0.9f, 1.0f};
    constexpr ImVec4 COL_WIN_BG{0.08f, 0.08f, 0.08f, 0.95f};
    constexpr ImVec4 COL_POPUP_BG{0.1f, 0.1f, 0.1f, 0.95f};
    constexpr ImVec4 COL_BORDER{0.2f, 0.2f, 0.2f, 1.0f};
    constexpr ImVec4 COL_FRAME{0.15f, 0.15f, 0.15f, 1.0f};
    constexpr ImVec4 COL_FRAME_HOV{0.2f, 0.2f, 0.2f, 1.0f};
    constexpr ImVec4 COL_FRAME_ACT{0.25f, 0.25f, 0.25f, 1.0f};
    constexpr ImVec4 COL_TITLE_BG{0.05f, 0.05f, 0.05f, 1.0f};
    constexpr ImVec4 COL_TITLE_ACT{0.1f, 0.1f, 0.1f, 1.0f};
    constexpr ImVec4 COL_BTN{0.2f, 0.2f, 0.2f, 1.0f};
    constexpr ImVec4 COL_BTN_HOV{0.3f, 0.3f, 0.3f, 1.0f};
    constexpr ImVec4 COL_BTN_ACT{0.4f, 0.4f, 0.4f, 1.0f};
    constexpr ImVec4 COL_HDR{0.25f, 0.25f, 0.25f, 1.0f};
    constexpr ImVec4 COL_HDR_HOV{0.35f, 0.35f, 0.35f, 1.0f};
    constexpr ImVec4 COL_HDR_ACT{0.45f, 0.45f, 0.45f, 1.0f};
    constexpr ImVec4 COL_TAB{0.15f, 0.15f, 0.15f, 1.0f};
    constexpr ImVec4 COL_TAB_HOV{0.3f, 0.3f, 0.3f, 1.0f};
    constexpr ImVec4 COL_TAB_ACT{0.25f, 0.25f, 0.25f, 1.0f};
    constexpr ImVec4 COL_CHECK{0.9f, 0.9f, 0.9f, 1.0f};
    constexpr ImVec4 COL_SLIDER{0.6f, 0.6f, 0.6f, 1.0f};
    constexpr ImVec4 COL_SLIDER_ACT{0.8f, 0.8f, 0.8f, 1.0f};
    constexpr ImVec4 COL_SEP{0.2f, 0.2f, 0.2f, 1.0f};

    constexpr float UI_DEFAULT_WINDOW_W = 450.0f;
    constexpr float UI_DEFAULT_WINDOW_H = 350.0f;
    constexpr float SNAPLINE_OUTLINE_ADD = 2.0f;
    constexpr float BOX_OUTLINE_ADD = 2.0f;
    constexpr int HP_BAR_BG_R = 40;
    constexpr int HP_BAR_BG_G = 40;
    constexpr int HP_BAR_BG_B = 40;
    constexpr int HP_BAR_BG_A = 200;
}

void render::setup_monochrome()
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
    c[ImGuiCol_Text] = consts::COL_TEXT;
    c[ImGuiCol_WindowBg] = consts::COL_WIN_BG;
    c[ImGuiCol_PopupBg] = consts::COL_POPUP_BG;
    c[ImGuiCol_Border] = consts::COL_BORDER;
    c[ImGuiCol_FrameBg] = consts::COL_FRAME;
    c[ImGuiCol_FrameBgHovered] = consts::COL_FRAME_HOV;
    c[ImGuiCol_FrameBgActive] = consts::COL_FRAME_ACT;
    c[ImGuiCol_TitleBg] = consts::COL_TITLE_BG;
    c[ImGuiCol_TitleBgActive] = consts::COL_TITLE_ACT;
    c[ImGuiCol_Button] = consts::COL_BTN;
    c[ImGuiCol_ButtonHovered] = consts::COL_BTN_HOV;
    c[ImGuiCol_ButtonActive] = consts::COL_BTN_ACT;
    c[ImGuiCol_Header] = consts::COL_HDR;
    c[ImGuiCol_HeaderHovered] = consts::COL_HDR_HOV;
    c[ImGuiCol_HeaderActive] = consts::COL_HDR_ACT;
    c[ImGuiCol_Tab] = consts::COL_TAB;
    c[ImGuiCol_TabHovered] = consts::COL_TAB_HOV;
    c[ImGuiCol_TabActive] = consts::COL_TAB_ACT;
    c[ImGuiCol_CheckMark] = consts::COL_CHECK;
    c[ImGuiCol_SliderGrab] = consts::COL_SLIDER;
    c[ImGuiCol_SliderGrabActive] = consts::COL_SLIDER_ACT;
    c[ImGuiCol_Separator] = consts::COL_SEP;
}

void render::draw_esp(ImDrawList *const dl, const std::vector<PlayerData> &players, const Config &cfg, int sw, int sh)
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
                snap_start = {sw / 2.0f, static_cast<float>(sh)};
            else if (cfg.snapline_type == 1)
                snap_start = {sw / 2.0f, sh / 2.0f};
            else
                snap_start = {sw / 2.0f, 0.0f};

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

void render::draw_menu(Config &cfg, bool &menu, char *cfg_input, std::string &cfg_name)
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
