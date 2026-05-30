#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "render.h"
#include "../config/config.h"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <unordered_map>
#include <cmath>

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
    constexpr float SNAPLINE_OUTLINE_ADD = 2.0f;
    constexpr float BOX_OUTLINE_ADD = 2.0f;
    constexpr int HP_BAR_BG_R = 40;
    constexpr int HP_BAR_BG_G = 40;
    constexpr int HP_BAR_BG_B = 40;
    constexpr int HP_BAR_BG_A = 200;
    constexpr ImU32 COL_BLACK = IM_COL32(0, 0, 0, 255);
    constexpr float UI_WINDOW_ROUNDING = 8.0f;
    constexpr float UI_ROUNDING = 4.0f;
    constexpr float UI_BORDER_SIZE = 1.0f;
    constexpr float UI_PAD = 15.0f;
    constexpr float UI_PAD_SMALL = 8.0f;
    constexpr float UI_SPACE = 10.0f;
    constexpr float UI_SPACE_SMALL = 5.0f;
    constexpr float UI_CHECK_SIZE = 16.0f;
    constexpr float UI_SLIDER_HEIGHT = 20.0f;
    constexpr float UI_SLIDER_WIDTH = 150.0f;
    constexpr float UI_BUTTON_HEIGHT = 24.0f;
    constexpr float UI_TAB_HEIGHT = 30.0f;
    constexpr float UI_SIDEBAR_WIDTH = 120.0f;
    constexpr float UI_CONTENT_WIDTH = 360.0f;
    constexpr float UI_WINDOW_W = 500.0f;
    constexpr float UI_WINDOW_H = 480.0f;
    constexpr float UI_FONT_SIZE = 16.0f;
    constexpr ImU32 UI_COL_BG = IM_COL32(20, 20, 20, 255);
    constexpr ImU32 UI_COL_CHILD = IM_COL32(25, 25, 25, 255);
    constexpr ImU32 UI_COL_FRAME = IM_COL32(30, 30, 30, 255);
    constexpr ImU32 UI_COL_BORDER = IM_COL32(60, 60, 60, 255);
    constexpr ImU32 UI_COL_ACCENT = IM_COL32(100, 150, 250, 255);
    constexpr ImU32 UI_COL_TEXT = IM_COL32(200, 200, 200, 255);
    constexpr ImU32 UI_COL_TEXT_DIM = IM_COL32(120, 120, 120, 255);
    constexpr ImU32 UI_COL_HOVER = IM_COL32(45, 45, 45, 255);
    constexpr ImU32 UI_COL_ACTIVE = IM_COL32(60, 60, 60, 255);
    constexpr float ANIM_SPEED_HOVER = 15.0f;
    constexpr float ANIM_SPEED_CHECK = 18.0f;
    constexpr float ANIM_SPEED_SLIDER = 20.0f;
    constexpr float ANIM_SPEED_TAB = 12.0f;
    constexpr float ANIM_SPEED_FADE = 8.0f;
    constexpr float ANIM_SPEED_SLIDE = 10.0f;
    constexpr float ANIM_SPEED_WATERMARK = 3.0f;
    constexpr float ANIM_SPEED_PULSE = 2.0f;
    constexpr float WATERMARK_PAD_X = 16.0f;
    constexpr float WATERMARK_PAD_Y = 10.0f;
    constexpr float WATERMARK_MARGIN = 20.0f;
    constexpr float WATERMARK_FONT_SIZE = 18.0f;
    constexpr float MAX_TEXT_WIDTH = 10000.0f;
    constexpr float TAB_INDICATOR_WIDTH = 3.0f;
    constexpr float CHECKBOX_INNER_PADDING = 6.0f;
    constexpr float ANIM_THRESHOLD = 0.001f;
    constexpr float PULSE_MIN = 0.85f;
    constexpr float PULSE_MAX = 1.0f;
}

namespace tr
{
    inline const char *enable(int lang) { return lang == 1 ? "ESP" : "ESP"; }
    inline const char *teammates(int lang) { return lang == 1 ? "Союзники" : "Teammates"; }
    inline const char *outline(int lang) { return lang == 1 ? "Обводка" : "Outline"; }
    inline const char *boxes(int lang) { return lang == 1 ? "Боксы" : "Boxes"; }
    inline const char *hp_bar(int lang) { return lang == 1 ? "Полоска HP" : "HP Bar"; }
    inline const char *names(int lang) { return lang == 1 ? "Имена" : "Names"; }
    inline const char *snaplines(int lang) { return lang == 1 ? "Линии" : "Lines"; }
    inline const char *frame(int lang) { return lang == 1 ? "Рамка" : "Frame"; }
    inline const char *corners(int lang) { return lang == 1 ? "Углы" : "Corners"; }
    inline const char *left(int lang) { return lang == 1 ? "Слева" : "Left"; }
    inline const char *right(int lang) { return lang == 1 ? "Справа" : "Right"; }
    inline const char *top(int lang) { return lang == 1 ? "Сверху" : "Top"; }
    inline const char *bottom(int lang) { return lang == 1 ? "Снизу" : "Bottom"; }
    inline const char *center(int lang) { return lang == 1 ? "Центр" : "Center"; }
    inline const char *t_team(int lang) { return lang == 1 ? "Т" : "T"; }
    inline const char *ct_team(int lang) { return lang == 1 ? "СТ" : "CT"; }
    inline const char *text(int lang) { return lang == 1 ? "Текст" : "Text"; }
    inline const char *options(int lang) { return lang == 1 ? "Настройки" : "Options"; }
    inline const char *configs(int lang) { return lang == 1 ? "Конфиги" : "Configs"; }
    inline const char *visuals(int lang) { return lang == 1 ? "Визуалы" : "Visuals"; }
    inline const char *save(int lang) { return lang == 1 ? "Сохранить" : "Save"; }
    inline const char *load(int lang) { return lang == 1 ? "Загрузить" : "Load"; }
    inline const char *del(int lang) { return lang == 1 ? "Удалить" : "Delete"; }
    inline const char *saved_configs(int lang) { return lang == 1 ? "Сохраненные конфиги:" : "Saved Configs:"; }
    inline const char *language(int lang) { return lang == 1 ? "Язык" : "Language"; }
}

namespace
{
    inline float ease_out_cubic(float t)
    {
        float t1 = 1.0f - t;
        return 1.0f - t1 * t1 * t1;
    }

    inline float ease_in_out_cubic(float t)
    {
        return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }

    inline float smooth_step(float edge0, float edge1, float x)
    {
        float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    inline ImVec4 u32_to_vec4(ImU32 c)
    {
        return ImVec4(
            ((c >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f,
            ((c >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f,
            ((c >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f,
            ((c >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f);
    }

    inline ImU32 lerp_color(ImU32 c1, ImU32 c2, float t)
    {
        if (t < 0.0f)
            t = 0.0f;
        if (t > 1.0f)
            t = 1.0f;
        ImVec4 v1 = u32_to_vec4(c1);
        ImVec4 v2 = u32_to_vec4(c2);
        return ImGui::ColorConvertFloat4ToU32(ImVec4(
            v1.x + (v2.x - v1.x) * t,
            v1.y + (v2.y - v1.y) * t,
            v1.z + (v2.z - v1.z) * t,
            v1.w + (v2.w - v1.w) * t));
    }

    inline ImU32 apply_alpha(ImU32 c, float a)
    {
        ImVec4 v = u32_to_vec4(c);
        v.w *= a;
        if (v.w < 0.0f)
            v.w = 0.0f;
        if (v.w > 1.0f)
            v.w = 1.0f;
        return ImGui::ColorConvertFloat4ToU32(v);
    }

    namespace anim
    {
        static std::unordered_map<ImGuiID, float> states;

        float update(ImGuiID id, float target, float speed = 10.0f)
        {
            float &val = states[id];
            float dt = ImGui::GetIO().DeltaTime;
            float diff = target - val;
            if (std::abs(diff) < consts::ANIM_THRESHOLD)
            {
                val = target;
                return val;
            }
            float eased_speed = speed * (1.0f + std::abs(diff) * 2.0f);
            val += diff * dt * eased_speed;
            return val;
        }

        float update_eased(ImGuiID id, float target, float speed = 10.0f)
        {
            float &val = states[id];
            float dt = ImGui::GetIO().DeltaTime;
            float diff = target - val;
            if (std::abs(diff) < consts::ANIM_THRESHOLD)
            {
                val = target;
                return val;
            }
            float step = diff * dt * speed;
            float eased_step = step * ease_out_cubic(std::abs(diff));
            val += eased_step;
            return val;
        }
    }

    namespace ui
    {
        void text(const char *label)
        {
            ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.00f), "%s", label);
        }

        bool checkbox(const char *label, bool *v)
        {
            ImGui::PushID(label);
            bool changed = false;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float size = consts::UI_CHECK_SIZE;
            float label_w = ImGui::CalcTextSize(label).x;
            if (ImGui::InvisibleButton("##cb", ImVec2(size + consts::UI_SPACE_SMALL + label_w, size)))
            {
                *v = !(*v);
                changed = true;
            }
            bool hovered = ImGui::IsItemHovered();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            ImVec2 c_min = pos;
            ImVec2 c_max = ImVec2(pos.x + size, pos.y + size);
            float rounding = consts::UI_ROUNDING;
            ImGuiID id = ImGui::GetID("##cb");
            float hover_anim = anim::update_eased(id + 1, hovered ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            float check_anim = anim::update_eased(id + 2, *v ? 1.0f : 0.0f, consts::ANIM_SPEED_CHECK);
            float pulse_time = static_cast<float>(ImGui::GetTime()) * consts::ANIM_SPEED_PULSE;
            float pulse = (*v && hovered) ? consts::PULSE_MIN + (consts::PULSE_MAX - consts::PULSE_MIN) * (std::sin(pulse_time) * 0.5f + 0.5f) : 1.0f;
            ImU32 border_col = lerp_color(consts::UI_COL_BORDER, consts::UI_COL_ACCENT, hover_anim);
            float scale = 1.0f + hover_anim * 0.1f;
            float scaled_size = size * scale * pulse;
            float offset = (size - scaled_size) / 2.0f;
            ImVec2 scaled_min = ImVec2(c_min.x + offset, c_min.y + offset);
            ImVec2 scaled_max = ImVec2(c_max.x - offset, c_max.y - offset);
            dl->AddRectFilled(scaled_min, scaled_max, consts::UI_COL_FRAME, rounding * scale);
            dl->AddRect(scaled_min, scaled_max, border_col, rounding * scale);
            if (check_anim > 0.01f)
            {
                float eased_check = ease_out_cubic(check_anim);
                float inner_size = (scaled_size - consts::CHECKBOX_INNER_PADDING) * eased_check;
                float inner_offset = (scaled_size - inner_size) / 2.0f;
                dl->AddRectFilled(
                    ImVec2(scaled_min.x + inner_offset, scaled_min.y + inner_offset),
                    ImVec2(scaled_max.x - inner_offset, scaled_max.y - inner_offset),
                    consts::UI_COL_ACCENT,
                    rounding * eased_check);
            }
            dl->AddText(ImVec2(c_max.x + consts::UI_SPACE_SMALL, pos.y), consts::UI_COL_TEXT, label);
            ImGui::PopID();
            return changed;
        }

        bool slider_float(const char *id, float *v, float min, float max)
        {
            ImGui::PushID(id);
            bool changed = false;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float width = consts::UI_SLIDER_WIDTH;
            float height = consts::UI_SLIDER_HEIGHT;
            if (ImGui::InvisibleButton("##sl", ImVec2(width, height)))
            {
            }
            bool active = ImGui::IsItemActive();
            bool hovered = ImGui::IsItemHovered();
            if (hovered && ImGui::IsMouseClicked(0))
            {
                float click_x = ImGui::GetIO().MousePos.x - pos.x;
                float ratio = click_x / width;
                *v = min + ratio * (max - min);
                changed = true;
            }
            if (active && ImGui::IsMouseDragging(0))
            {
                float click_x = ImGui::GetIO().MousePos.x - pos.x;
                float ratio = click_x / width;
                *v = min + ratio * (max - min);
                changed = true;
            }
            if (*v < min)
                *v = min;
            if (*v > max)
                *v = max;
            ImDrawList *dl = ImGui::GetWindowDrawList();
            float ratio = (*v - min) / (max - min);
            float rounding = consts::UI_ROUNDING;
            ImGuiID sid = ImGui::GetID("##sl");
            float hover_anim = anim::update_eased(sid + 1, hovered || active ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            float anim_ratio = anim::update(sid + 2, ratio, consts::ANIM_SPEED_SLIDER);
            float eased_ratio = ease_out_cubic(anim_ratio);
            ImU32 border_col = lerp_color(consts::UI_COL_BORDER, consts::UI_COL_ACCENT, hover_anim);
            float scale = 1.0f + hover_anim * 0.05f;
            float scaled_height = height * scale;
            float y_offset = (height - scaled_height) / 2.0f;
            ImVec2 scaled_min = ImVec2(pos.x, pos.y + y_offset);
            ImVec2 scaled_max = ImVec2(pos.x + width, pos.y + y_offset + scaled_height);
            dl->AddRectFilled(scaled_min, scaled_max, consts::UI_COL_FRAME, rounding * scale);
            dl->AddRectFilled(scaled_min, ImVec2(scaled_min.x + width * eased_ratio, scaled_max.y), consts::UI_COL_ACCENT, rounding * scale);
            dl->AddRect(scaled_min, scaled_max, border_col, rounding * scale);
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.1f", *v);
            ImVec2 val_sz = ImGui::CalcTextSize(buf);
            dl->AddText(ImVec2(scaled_min.x + width - val_sz.x - consts::UI_PAD_SMALL, scaled_min.y + (scaled_height - val_sz.y) / 2.0f), consts::UI_COL_TEXT, buf);
            ImGui::PopID();
            return changed;
        }

        bool combo(const char *id, int *current, const char *const items[], int count)
        {
            ImGui::PushID(id);
            bool changed = false;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float width = consts::UI_SLIDER_WIDTH;
            float height = consts::UI_BUTTON_HEIGHT;
            if (ImGui::InvisibleButton("##combo", ImVec2(width, height)))
            {
                ImGui::OpenPopup(id);
            }
            bool hovered = ImGui::IsItemHovered();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            float rounding = consts::UI_ROUNDING;
            ImGuiID cid = ImGui::GetID("##combo");
            float hover_anim = anim::update_eased(cid + 1, hovered ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            ImU32 border_col = lerp_color(consts::UI_COL_BORDER, consts::UI_COL_ACCENT, hover_anim);
            float scale = 1.0f + hover_anim * 0.05f;
            float scaled_height = height * scale;
            float y_offset = (height - scaled_height) / 2.0f;
            ImVec2 scaled_min = ImVec2(pos.x, pos.y + y_offset);
            ImVec2 scaled_max = ImVec2(pos.x + width, pos.y + y_offset + scaled_height);
            dl->AddRectFilled(scaled_min, scaled_max, consts::UI_COL_FRAME, rounding * scale);
            dl->AddRect(scaled_min, scaled_max, border_col, rounding * scale);
            ImVec2 txt_sz = ImGui::CalcTextSize(items[*current]);
            dl->AddText(ImVec2(scaled_min.x + consts::UI_PAD_SMALL, scaled_min.y + (scaled_height - txt_sz.y) / 2.0f), consts::UI_COL_TEXT, items[*current]);
            if (ImGui::BeginPopup(id))
            {
                for (int i = 0; i < count; i++)
                {
                    if (ImGui::Selectable(items[i], i == *current))
                    {
                        *current = i;
                        changed = true;
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::PopID();
            return changed;
        }

        bool button(const char *label, float width)
        {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 size = ImVec2(width, consts::UI_BUTTON_HEIGHT);
            if (width == 0.0f)
                size.x = ImGui::CalcTextSize(label).x + consts::UI_PAD * 2.0f;
            if (ImGui::InvisibleButton(label, size))
                return true;
            bool hovered = ImGui::IsItemHovered();
            bool active = ImGui::IsItemActive();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            float rounding = consts::UI_ROUNDING;
            ImGuiID bid = ImGui::GetID(label);
            float hover_anim = anim::update_eased(bid + 1, hovered ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            float active_anim = anim::update_eased(bid + 2, active ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            ImU32 bg = lerp_color(consts::UI_COL_FRAME, consts::UI_COL_HOVER, hover_anim);
            bg = lerp_color(bg, consts::UI_COL_ACCENT, active_anim);
            ImU32 border_col = lerp_color(consts::UI_COL_BORDER, consts::UI_COL_ACCENT, hover_anim + active_anim);
            float scale = 1.0f + hover_anim * 0.08f - active_anim * 0.05f;
            float scaled_w = size.x * scale;
            float scaled_h = size.y * scale;
            float x_offset = (size.x - scaled_w) / 2.0f;
            float y_offset = (size.y - scaled_h) / 2.0f;
            ImVec2 scaled_min = ImVec2(pos.x + x_offset, pos.y + y_offset);
            ImVec2 scaled_max = ImVec2(pos.x + x_offset + scaled_w, pos.y + y_offset + scaled_h);
            dl->AddRectFilled(scaled_min, scaled_max, bg, rounding * scale);
            dl->AddRect(scaled_min, scaled_max, border_col, rounding * scale);
            ImVec2 text_size = ImGui::CalcTextSize(label);
            dl->AddText(ImVec2(scaled_min.x + (scaled_w - text_size.x) / 2.0f, scaled_min.y + (scaled_h - text_size.y) / 2.0f), consts::UI_COL_TEXT, label);
            return false;
        }

        bool tab(const char *label, bool active)
        {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, consts::UI_TAB_HEIGHT);
            if (ImGui::InvisibleButton(label, size))
                return true;
            bool hovered = ImGui::IsItemHovered();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            float rounding = consts::UI_ROUNDING;
            ImGuiID tid = ImGui::GetID(label);
            float hover_anim = anim::update_eased(tid + 1, hovered ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            float active_anim = anim::update_eased(tid + 2, active ? 1.0f : 0.0f, consts::ANIM_SPEED_TAB);
            ImU32 bg = lerp_color(consts::UI_COL_FRAME, consts::UI_COL_HOVER, hover_anim);
            bg = lerp_color(bg, consts::UI_COL_ACTIVE, active_anim);
            float scale = 1.0f + hover_anim * 0.03f;
            float scaled_w = size.x * scale;
            float x_offset = (size.x - scaled_w) / 2.0f;
            ImVec2 scaled_min = ImVec2(pos.x + x_offset, pos.y);
            ImVec2 scaled_max = ImVec2(pos.x + x_offset + scaled_w, pos.y + size.y);
            dl->AddRectFilled(scaled_min, scaled_max, bg, rounding * scale);
            if (active_anim > 0.01f)
            {
                float eased_active = ease_out_cubic(active_anim);
                float ind_w = consts::TAB_INDICATOR_WIDTH * eased_active;
                dl->AddRectFilled(scaled_min, ImVec2(scaled_min.x + ind_w, scaled_max.y), consts::UI_COL_ACCENT, 0.0f);
            }
            ImVec2 text_size = ImGui::CalcTextSize(label);
            ImU32 text_col = lerp_color(consts::UI_COL_TEXT_DIM, consts::UI_COL_TEXT, active_anim);
            dl->AddText(ImVec2(scaled_min.x + consts::UI_PAD, scaled_min.y + (size.y - text_size.y) / 2.0f), text_col, label);
            return false;
        }

        bool input_text(const char *id, char *buf, std::size_t size)
        {
            ImGui::PushID(id);
            bool changed = ImGui::InputText("##input", buf, size);
            ImGui::PopID();
            return changed;
        }

        bool color_edit(const char *label, float col[4])
        {
            ImGui::PushID(label);
            text(label);
            bool changed = ImGui::ColorEdit4("##color", col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::PopID();
            return changed;
        }

        bool selectable(const char *label, bool selected)
        {
            return ImGui::Selectable(label, selected);
        }
    }

    void draw_snapline(ImDrawList *const dl, const ImVec4 &col, const Vector3 &feet, const Config &cfg, int sw, int sh)
    {
        ImVec2 start;
        if (cfg.snapline_type == 0)
            start = ImVec2(static_cast<float>(sw) / consts::BOX_WIDTH_DIVISOR, static_cast<float>(sh));
        else if (cfg.snapline_type == 1)
            start = ImVec2(static_cast<float>(sw) / consts::BOX_WIDTH_DIVISOR, static_cast<float>(sh) / consts::BOX_WIDTH_DIVISOR);
        else
            start = ImVec2(static_cast<float>(sw) / consts::BOX_WIDTH_DIVISOR, 0.0f);
        const ImVec2 end(feet.x, feet.y);
        const ImU32 clr = ImGui::ColorConvertFloat4ToU32(col);
        if (cfg.esp_outline)
            dl->AddLine(start, end, consts::COL_BLACK, cfg.snapline_border + consts::SNAPLINE_OUTLINE_ADD);
        dl->AddLine(start, end, clr, cfg.snapline_border);
    }

    void draw_box(ImDrawList *const dl, const ImVec4 &col, float l, float t, float r, float b, float w, float h, const Config &cfg)
    {
        const ImU32 clr = ImGui::ColorConvertFloat4ToU32(col);
        if (cfg.box_type == 0)
        {
            if (cfg.esp_outline)
                dl->AddRect(ImVec2(l - consts::BOX_PADDING, t - consts::BOX_PADDING), ImVec2(r + consts::BOX_PADDING, b + consts::BOX_PADDING), consts::COL_BLACK);
            dl->AddRect(ImVec2(l, t), ImVec2(r, b), clr, 0.0f, 0, cfg.box_border);
        }
        else
        {
            auto draw_c = [&](ImU32 c, float th)
            {
                const float lw = w / consts::CORNER_DIVISOR;
                const float lh = h / consts::CORNER_DIVISOR;
                dl->AddLine(ImVec2(l, t), ImVec2(l + lw, t), c, th);
                dl->AddLine(ImVec2(l, t), ImVec2(l, t + lh), c, th);
                dl->AddLine(ImVec2(r, t), ImVec2(r - lw, t), c, th);
                dl->AddLine(ImVec2(r, t), ImVec2(r, t + lh), c, th);
                dl->AddLine(ImVec2(l, b), ImVec2(l + lw, b), c, th);
                dl->AddLine(ImVec2(l, b), ImVec2(l, b - lh), c, th);
                dl->AddLine(ImVec2(r, b), ImVec2(r - lw, b), c, th);
                dl->AddLine(ImVec2(r, b), ImVec2(r, b - lh), c, th);
            };
            if (cfg.esp_outline)
                draw_c(consts::COL_BLACK, cfg.box_border + consts::BOX_OUTLINE_ADD);
            draw_c(clr, cfg.box_border);
        }
    }

    void draw_health(ImDrawList *const dl, int health, float l, float t, float r, float b, float w, float h, const Config &cfg)
    {
        const float pct = std::clamp(static_cast<float>(health) / static_cast<float>(consts::HEALTH_DIVISOR), consts::MIN_HEALTH_PERCENT, consts::MAX_HEALTH_PERCENT);
        const ImU32 hclr = IM_COL32(static_cast<int>((consts::MAX_HEALTH_PERCENT - pct) * consts::BYTE_MAX), static_cast<int>(pct * consts::BYTE_MAX), 0, consts::BYTE_MAX);
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
            dl->AddRectFilled(ImVec2(bl - consts::BOX_PADDING, bt - consts::BOX_PADDING), ImVec2(br + consts::BOX_PADDING, bb + consts::BOX_PADDING), consts::COL_BLACK);
        dl->AddRectFilled(ImVec2(bl, bt), ImVec2(br, bb), IM_COL32(consts::HP_BAR_BG_R, consts::HP_BAR_BG_G, consts::HP_BAR_BG_B, consts::HP_BAR_BG_A));
        dl->AddRectFilled(ImVec2(fl, ft), ImVec2(fr, fb), hclr);
    }

    void draw_name(ImDrawList *const dl, const std::string &name, const ImVec4 &col_txt, float l, float t, float r, float b, float w, float h, const Config &cfg)
    {
        const ImVec2 sz = ImGui::CalcTextSize(name.c_str());
        ImVec2 pos;
        if (cfg.name_pos == 0)
            pos = ImVec2(l + w / consts::BOX_WIDTH_DIVISOR - sz.x / consts::BOX_WIDTH_DIVISOR, t - sz.y - consts::TEXT_PADDING);
        else if (cfg.name_pos == 1)
            pos = ImVec2(l + w / consts::BOX_WIDTH_DIVISOR - sz.x / consts::BOX_WIDTH_DIVISOR, b + consts::TEXT_PADDING);
        else if (cfg.name_pos == 2)
            pos = ImVec2(l - sz.x - consts::TEXT_PADDING, t + h / consts::BOX_WIDTH_DIVISOR - sz.y / consts::BOX_WIDTH_DIVISOR);
        else
            pos = ImVec2(r + consts::TEXT_PADDING, t + h / consts::BOX_WIDTH_DIVISOR - sz.y / consts::BOX_WIDTH_DIVISOR);
        const ImU32 txt_clr = ImGui::ColorConvertFloat4ToU32(col_txt);
        if (cfg.esp_outline)
        {
            dl->AddText(ImVec2(pos.x - consts::BOX_PADDING, pos.y), consts::COL_BLACK, name.c_str());
            dl->AddText(ImVec2(pos.x + consts::BOX_PADDING, pos.y), consts::COL_BLACK, name.c_str());
            dl->AddText(ImVec2(pos.x, pos.y - consts::BOX_PADDING), consts::COL_BLACK, name.c_str());
            dl->AddText(ImVec2(pos.x, pos.y + consts::BOX_PADDING), consts::COL_BLACK, name.c_str());
        }
        dl->AddText(pos, txt_clr, name.c_str());
    }
}

void render::setup_monochrome()
{
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", consts::UI_FONT_SIZE);
    ImGuiStyle &s = ImGui::GetStyle();
    s.WindowRounding = consts::UI_WINDOW_ROUNDING;
    s.ChildRounding = consts::UI_WINDOW_ROUNDING;
    s.FrameRounding = consts::UI_ROUNDING;
    s.PopupRounding = consts::UI_ROUNDING;
    s.GrabRounding = consts::UI_ROUNDING;
    s.TabRounding = consts::UI_ROUNDING;
    s.WindowBorderSize = consts::UI_BORDER_SIZE;
    s.FrameBorderSize = 0.0f;
    s.PopupBorderSize = consts::UI_BORDER_SIZE;
    s.WindowPadding = ImVec2(consts::UI_PAD, consts::UI_PAD);
    s.FramePadding = ImVec2(consts::UI_PAD_SMALL, consts::UI_PAD_SMALL);
    s.ItemSpacing = ImVec2(consts::UI_SPACE, consts::UI_SPACE);
    s.ItemInnerSpacing = ImVec2(consts::UI_SPACE_SMALL, consts::UI_SPACE_SMALL);
    ImVec4 *c = s.Colors;
    c[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    c[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    c[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    c[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    c[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    c[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    c[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    c[ImGuiCol_CheckMark] = ImVec4(0.39f, 0.58f, 0.98f, 1.00f);
    c[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.58f, 0.98f, 1.00f);
    c[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.58f, 0.98f, 1.00f);
    c[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    c[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
}

void render::draw_esp(ImDrawList *const dl, const std::vector<PlayerData> &players, const Config &cfg, int sw, int sh)
{
    if (!cfg.esp_enabled || players.empty())
        return;
    const ImVec4 col_t(cfg.color_t[0], cfg.color_t[1], cfg.color_t[2], cfg.color_t[3]);
    const ImVec4 col_ct(cfg.color_ct[0], cfg.color_ct[1], cfg.color_ct[2], cfg.color_t[3]);
    const ImVec4 col_txt(cfg.color_txt[0], cfg.color_txt[1], cfg.color_txt[2], cfg.color_txt[3]);
    for (const auto &p : players)
    {
        if (!p.is_on_screen)
            continue;
        const float h = p.feet_screen.y - p.head_screen.y;
        if (h <= 0.0f)
            continue;
        const float w = h / consts::BOX_WIDTH_DIVISOR;
        const float l = p.head_screen.x - w / consts::BOX_WIDTH_DIVISOR;
        const float t = p.head_screen.y;
        const float r = l + w;
        const float b = p.feet_screen.y;
        const ImVec4 &team_col = (p.team == consts::TEAM_TERRORIST) ? col_t : col_ct;
        if (cfg.show_snaplines)
            draw_snapline(dl, team_col, p.feet_screen, cfg, sw, sh);
        if (cfg.show_boxes)
            draw_box(dl, team_col, l, t, r, b, w, h, cfg);
        if (cfg.show_health)
            draw_health(dl, p.health, l, t, r, b, w, h, cfg);
        if (cfg.show_names)
            draw_name(dl, p.name, col_txt, l, t, r, b, w, h, cfg);
    }
}

void render::draw_menu(Config &cfg, bool &menu, char *cfg_input, std::string &cfg_name)
{
    static int active_tab = 0;
    static int last_tab = -1;
    static float fade_alpha = 0.0f;
    static float slide_offset = 0.0f;
    static bool menu_visible = false;
    static float menu_alpha = 0.0f;
    if (menu && !menu_visible)
    {
        menu_visible = true;
    }
    else if (!menu && menu_visible)
    {
        menu_alpha -= ImGui::GetIO().DeltaTime * consts::ANIM_SPEED_FADE;
        if (menu_alpha <= 0.0f)
        {
            menu_alpha = 0.0f;
            menu_visible = false;
        }
    }
    if (menu_visible)
    {
        menu_alpha += ImGui::GetIO().DeltaTime * consts::ANIM_SPEED_FADE;
        if (menu_alpha > 1.0f)
            menu_alpha = 1.0f;
    }
    if (last_tab != active_tab)
    {
        fade_alpha = 0.0f;
        slide_offset = (active_tab > last_tab) ? 20.0f : -20.0f;
        last_tab = active_tab;
    }
    fade_alpha += ImGui::GetIO().DeltaTime * consts::ANIM_SPEED_FADE;
    if (fade_alpha > 1.0f)
        fade_alpha = 1.0f;
    slide_offset *= 0.85f;
    if (std::abs(slide_offset) < 0.1f)
        slide_offset = 0.0f;
    float eased_alpha = ease_out_cubic(menu_alpha);
    if (eased_alpha < 0.01f)
        return;
    ImGui::SetNextWindowSize(ImVec2(consts::UI_WINDOW_W, consts::UI_WINDOW_H), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, eased_alpha);
    ImGui::Begin("##VorExternalMain", &menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (ImGui::BeginTable("##MainLayout", 2, ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("##Sidebar", ImGuiTableColumnFlags_WidthFixed, consts::UI_SIDEBAR_WIDTH);
        ImGui::TableSetupColumn("##Content", ImGuiTableColumnFlags_WidthFixed, consts::UI_CONTENT_WIDTH);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::BeginChild("##SidebarChild", ImVec2(0, 0), true);
        if (ui::tab(tr::visuals(cfg.language), active_tab == 0))
            active_tab = 0;
        if (ui::tab(tr::options(cfg.language), active_tab == 1))
            active_tab = 1;
        if (ui::tab(tr::configs(cfg.language), active_tab == 2))
            active_tab = 2;
        ImGui::EndChild();
        ImGui::TableNextColumn();
        ImGui::BeginChild("##ContentChild", ImVec2(0, 0), true);
        float content_alpha = ease_out_cubic(fade_alpha);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, content_alpha);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + slide_offset);
        if (active_tab == 0)
        {
            if (ui::checkbox(tr::enable(cfg.language), &cfg.esp_enabled))
            {
            }
            if (cfg.esp_enabled)
            {
                ImGui::Indent();
                ui::checkbox(tr::teammates(cfg.language), &cfg.esp_teammates);
                ui::checkbox(tr::outline(cfg.language), &cfg.esp_outline);
                ui::checkbox(tr::boxes(cfg.language), &cfg.show_boxes);
                if (cfg.show_boxes)
                {
                    ImGui::Indent();
                    const char *bt[] = {tr::frame(cfg.language), tr::corners(cfg.language)};
                    ui::combo("##BoxType", &cfg.box_type, bt, 2);
                    ui::slider_float("##BoxThick", &cfg.box_border, consts::SLIDER_MIN, consts::SLIDER_MAX);
                    ImGui::Unindent();
                }
                ui::checkbox(tr::hp_bar(cfg.language), &cfg.show_health);
                if (cfg.show_health)
                {
                    ImGui::Indent();
                    const char *hp[] = {tr::left(cfg.language), tr::right(cfg.language), tr::top(cfg.language), tr::bottom(cfg.language)};
                    ui::combo("##HpPos", &cfg.hp_bar_pos, hp, 4);
                    ImGui::Unindent();
                }
                ui::checkbox(tr::names(cfg.language), &cfg.show_names);
                if (cfg.show_names)
                {
                    ImGui::Indent();
                    const char *np[] = {tr::top(cfg.language), tr::bottom(cfg.language), tr::left(cfg.language), tr::right(cfg.language)};
                    ui::combo("##NamePos", &cfg.name_pos, np, 4);
                    ImGui::Unindent();
                }
                ui::checkbox(tr::snaplines(cfg.language), &cfg.show_snaplines);
                if (cfg.show_snaplines)
                {
                    ImGui::Indent();
                    const char *st[] = {tr::bottom(cfg.language), tr::center(cfg.language), tr::top(cfg.language)};
                    ui::combo("##SnapOrigin", &cfg.snapline_type, st, 3);
                    ui::slider_float("##SnapThick", &cfg.snapline_border, consts::SLIDER_MIN, consts::SLIDER_MAX);
                    ImGui::Unindent();
                }
                ImGui::Unindent();
            }
        }
        else if (active_tab == 1)
        {
            const char *langs[] = {"English", "Русский"};
            ui::combo("##Lang", &cfg.language, langs, 2);
            ui::color_edit(tr::t_team(cfg.language), cfg.color_t);
            ui::color_edit(tr::ct_team(cfg.language), cfg.color_ct);
            ui::color_edit(tr::text(cfg.language), cfg.color_txt);
        }
        else if (active_tab == 2)
        {
            ui::input_text("##CfgName", cfg_input, consts::MAX_CONFIG_NAME_LENGTH);
            cfg_name = cfg_input;
            float btn_w = (ImGui::GetContentRegionAvail().x - consts::UI_SPACE * 2.0f) / 3.0f;
            if (ui::button(tr::save(cfg.language), btn_w))
                config_manager::save(cfg_name, cfg);
            ImGui::SameLine();
            if (ui::button(tr::load(cfg.language), btn_w))
                config_manager::load(cfg_name, cfg);
            ImGui::SameLine();
            if (ui::button(tr::del(cfg.language), btn_w))
                config_manager::remove(cfg_name);
            ImGui::Separator();
            ui::text(tr::saved_configs(cfg.language));
            ImGui::BeginChild("##ConfigList", ImVec2(0, 100.0f), true);
            for (const auto &n : config_manager::list())
            {
                if (ui::selectable(n.c_str(), cfg_name == n))
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
        }
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::EndTable();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void render::draw_watermark(ImDrawList *const dl)
{
    static float alpha = 0.0f;
    alpha += ImGui::GetIO().DeltaTime * consts::ANIM_SPEED_WATERMARK;
    if (alpha > 1.0f)
        alpha = 1.0f;
    float eased_alpha = ease_out_cubic(alpha);
    if (eased_alpha < 0.01f)
        return;
    const char *wm_text = "VorExternal";
    ImFont *font = ImGui::GetFont();
    ImVec2 text_size = font->CalcTextSizeA(consts::WATERMARK_FONT_SIZE, consts::MAX_TEXT_WIDTH, 0.0f, wm_text);
    float w = text_size.x + consts::WATERMARK_PAD_X * 2.0f;
    float h = text_size.y + consts::WATERMARK_PAD_Y * 2.0f;
    float rounding = h / 2.0f;
    ImVec2 screen_size = ImGui::GetIO().DisplaySize;
    ImVec2 pos(screen_size.x - w - consts::WATERMARK_MARGIN, consts::WATERMARK_MARGIN);
    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), apply_alpha(consts::UI_COL_FRAME, eased_alpha), rounding);
    dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), apply_alpha(consts::UI_COL_BORDER, eased_alpha), rounding);
    dl->AddText(font, consts::WATERMARK_FONT_SIZE, ImVec2(pos.x + consts::WATERMARK_PAD_X, pos.y + consts::WATERMARK_PAD_Y), apply_alpha(consts::UI_COL_ACCENT, eased_alpha), wm_text);
}
