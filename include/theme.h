#pragma once
#include "imgui.h"
#include <string>
#include <vector>

struct AccentColor {
    std::string name;
    ImVec4 color;
};

struct ThemeColors {
    ImVec4 background;
    ImVec4 surface;
    ImVec4 surface_hover;
    ImVec4 border;
    ImVec4 text;
    ImVec4 text_dim;
    ImVec4 accent;
    ImVec4 accent_hover;
    ImVec4 success;
    ImVec4 warning;
    ImVec4 error;
    ImVec4 info;
    ImVec4 console_bg;
    ImVec4 editor_bg;
    ImVec4 tab_bg;
    ImVec4 tab_active;
    ImVec4 scrollbar;
    ImVec4 header;
};

class Theme {
public:
    static void init();
    static void apply();
    static void set_accent(int index);
    static void set_background(int index);
    static void set_transparency(float alpha);

    static const std::vector<AccentColor>& get_accents();
    static const std::vector<AccentColor>& get_backgrounds();
    static ThemeColors& colors();
    static int current_accent;
    static int current_background;
    static float transparency;

private:
    static ThemeColors colors_;
    static std::vector<AccentColor> accents_;
    static std::vector<AccentColor> backgrounds_;
    static void rebuild_colors();
};
