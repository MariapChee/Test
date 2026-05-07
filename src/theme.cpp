#include "theme.h"

ThemeColors Theme::colors_;
int Theme::current_accent = 0;
int Theme::current_background = 0;
float Theme::transparency = 1.0f;
std::vector<AccentColor> Theme::accents_;
std::vector<AccentColor> Theme::backgrounds_;

void Theme::init() {
    accents_ = {
        {"Cyan",    ImVec4(0.0f, 1.0f, 1.0f, 1.0f)},
        {"Blue",    ImVec4(0.26f, 0.53f, 0.96f, 1.0f)},
        {"Purple",  ImVec4(0.58f, 0.29f, 0.92f, 1.0f)},
        {"Pink",    ImVec4(0.92f, 0.29f, 0.58f, 1.0f)},
        {"Red",     ImVec4(0.92f, 0.26f, 0.26f, 1.0f)},
        {"Orange",  ImVec4(0.96f, 0.52f, 0.09f, 1.0f)},
        {"Yellow",  ImVec4(0.96f, 0.82f, 0.09f, 1.0f)},
        {"Green",   ImVec4(0.18f, 0.72f, 0.36f, 1.0f)},
        {"Teal",    ImVec4(0.13f, 0.69f, 0.60f, 1.0f)},
        {"Indigo",  ImVec4(0.40f, 0.35f, 0.87f, 1.0f)},
    };

    backgrounds_ = {
        {"Dark",     ImVec4(0.06f, 0.06f, 0.09f, 1.0f)},
        {"Darker",   ImVec4(0.04f, 0.04f, 0.06f, 1.0f)},
        {"Navy",     ImVec4(0.08f, 0.10f, 0.18f, 1.0f)},
        {"Slate",    ImVec4(0.14f, 0.16f, 0.20f, 1.0f)},
        {"Charcoal", ImVec4(0.12f, 0.12f, 0.12f, 1.0f)},
        {"Midnight", ImVec4(0.02f, 0.02f, 0.05f, 1.0f)},
        {"Obsidian", ImVec4(0.05f, 0.05f, 0.07f, 1.0f)},
    };

    rebuild_colors();
}

void Theme::rebuild_colors() {
    ImVec4 bg = backgrounds_[current_background].color;
    ImVec4 ac = accents_[current_accent].color;

    colors_.background = bg;
    colors_.surface = ImVec4(bg.x + 0.03f, bg.y + 0.03f, bg.z + 0.05f, transparency);
    colors_.surface_hover = ImVec4(bg.x + 0.06f, bg.y + 0.06f, bg.z + 0.09f, transparency);
    colors_.border = ImVec4(ac.x * 0.3f, ac.y * 0.3f, ac.z * 0.3f, 0.5f);
    colors_.text = ImVec4(0.92f, 0.93f, 0.95f, 1.0f);
    colors_.text_dim = ImVec4(0.55f, 0.58f, 0.65f, 1.0f);
    colors_.accent = ac;
    colors_.accent_hover = ImVec4(ac.x * 0.8f, ac.y * 0.8f, ac.z * 0.8f, 1.0f);
    colors_.success = ImVec4(0.18f, 0.82f, 0.45f, 1.0f);
    colors_.warning = ImVec4(0.96f, 0.76f, 0.09f, 1.0f);
    colors_.error = ImVec4(0.92f, 0.26f, 0.26f, 1.0f);
    colors_.info = ImVec4(0.26f, 0.53f, 0.96f, 1.0f);
    colors_.console_bg = ImVec4(bg.x * 0.7f, bg.y * 0.7f, bg.z * 0.7f, transparency);
    colors_.editor_bg = ImVec4(bg.x + 0.01f, bg.y + 0.01f, bg.z + 0.02f, transparency);
    colors_.tab_bg = ImVec4(bg.x + 0.02f, bg.y + 0.02f, bg.z + 0.04f, transparency);
    colors_.tab_active = ImVec4(ac.x * 0.15f, ac.y * 0.15f, ac.z * 0.15f, transparency);
    colors_.scrollbar = ImVec4(ac.x * 0.4f, ac.y * 0.4f, ac.z * 0.4f, 0.5f);
    colors_.header = ImVec4(bg.x + 0.04f, bg.y + 0.04f, bg.z + 0.06f, transparency);
}

void Theme::apply() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.TabRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.ChildRounding = 6.0f;
    style.PopupRounding = 6.0f;

    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 5);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 8.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;

    ImVec4* c = style.Colors;
    c[ImGuiCol_WindowBg] = colors_.background;
    c[ImGuiCol_ChildBg] = colors_.surface;
    c[ImGuiCol_PopupBg] = ImVec4(colors_.surface.x, colors_.surface.y, colors_.surface.z, 0.98f);
    c[ImGuiCol_Border] = colors_.border;
    c[ImGuiCol_FrameBg] = colors_.surface;
    c[ImGuiCol_FrameBgHovered] = colors_.surface_hover;
    c[ImGuiCol_FrameBgActive] = colors_.surface_hover;
    c[ImGuiCol_TitleBg] = colors_.header;
    c[ImGuiCol_TitleBgActive] = colors_.header;
    c[ImGuiCol_MenuBarBg] = colors_.header;
    c[ImGuiCol_ScrollbarBg] = colors_.surface;
    c[ImGuiCol_ScrollbarGrab] = colors_.scrollbar;
    c[ImGuiCol_ScrollbarGrabHovered] = colors_.accent;
    c[ImGuiCol_ScrollbarGrabActive] = colors_.accent;
    c[ImGuiCol_CheckMark] = colors_.accent;
    c[ImGuiCol_SliderGrab] = colors_.accent;
    c[ImGuiCol_SliderGrabActive] = colors_.accent_hover;
    c[ImGuiCol_Button] = colors_.surface;
    c[ImGuiCol_ButtonHovered] = colors_.surface_hover;
    c[ImGuiCol_ButtonActive] = ImVec4(colors_.accent.x * 0.2f, colors_.accent.y * 0.2f, colors_.accent.z * 0.2f, 1.0f);
    c[ImGuiCol_Header] = colors_.surface;
    c[ImGuiCol_HeaderHovered] = colors_.surface_hover;
    c[ImGuiCol_HeaderActive] = colors_.tab_active;
    c[ImGuiCol_Separator] = colors_.border;
    c[ImGuiCol_Tab] = colors_.tab_bg;
    c[ImGuiCol_TabHovered] = colors_.surface_hover;
    c[ImGuiCol_TabSelected] = colors_.tab_active;
    c[ImGuiCol_TabDimmed] = colors_.tab_bg;
    c[ImGuiCol_TabDimmedSelected] = colors_.tab_active;
    c[ImGuiCol_ResizeGrip] = colors_.accent;
    c[ImGuiCol_ResizeGripHovered] = colors_.accent_hover;
    c[ImGuiCol_ResizeGripActive] = colors_.accent;
    c[ImGuiCol_Text] = colors_.text;
    c[ImGuiCol_TextDisabled] = colors_.text_dim;
    c[ImGuiCol_PlotHistogram] = colors_.accent;
    c[ImGuiCol_TableHeaderBg] = colors_.header;
    c[ImGuiCol_TableBorderStrong] = colors_.border;
    c[ImGuiCol_TableBorderLight] = ImVec4(colors_.border.x, colors_.border.y, colors_.border.z, 0.3f);
}

void Theme::set_accent(int index) {
    if (index >= 0 && index < (int)accents_.size()) {
        current_accent = index;
        rebuild_colors();
    }
}

void Theme::set_background(int index) {
    if (index >= 0 && index < (int)backgrounds_.size()) {
        current_background = index;
        rebuild_colors();
    }
}

void Theme::set_transparency(float alpha) {
    transparency = alpha;
    rebuild_colors();
}

const std::vector<AccentColor>& Theme::get_accents() { return accents_; }
const std::vector<AccentColor>& Theme::get_backgrounds() { return backgrounds_; }
ThemeColors& Theme::colors() { return colors_; }
