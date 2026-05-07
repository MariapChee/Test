#pragma once
#include <string>
#include <functional>
#include "imgui.h"

struct ExecutorSettings {
    bool auto_execute = false;
    bool auto_save = true;
    float font_size = 14.0f;
    bool top_most = false;
    bool unlock_fps = false;
    int target_fps = 60;
    bool save_console = true;
    bool word_wrap = true;
    bool line_numbers = true;
    bool auto_indent = true;
    float transparency = 1.0f;
    int theme_index = 0;
    int accent_index = 0;
    int background_index = 0;
    int language_index = 0;
    int max_console_logs = 5000;
    bool show_fps = true;
    bool confirm_execute = false;
    bool minimize_to_tray = false;
    std::string ai_api_key;
};

struct LanguageInfo {
    std::string code;
    std::string name;
    std::string native_name;
};

class Settings {
public:
    Settings();

    void render(float width, float height, std::function<void()> on_change = nullptr);

    ExecutorSettings& get();
    const ExecutorSettings& get() const;

    void load_from_disk();
    void save_to_disk();

    const LanguageInfo& current_language() const;
    static const std::vector<LanguageInfo>& get_languages();

private:
    ExecutorSettings settings_;
    static std::vector<LanguageInfo> languages_;
    char api_key_buf_[256] = "";
    bool show_api_key_ = false;

    std::string get_save_path() const;
    void render_general(float width);
    void render_editor_settings(float width);
    void render_appearance(float width);
    void render_advanced(float width);
    void render_language(float width);
    void render_about(float width);
};
