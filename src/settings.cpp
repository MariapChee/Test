#include "settings.h"
#include "theme.h"
#include "utils.h"
#include <json.hpp>
#include <cstring>

using json = nlohmann::json;

std::vector<LanguageInfo> Settings::languages_ = {
    {"en", "English", "English"},
    {"es", "Spanish", "Espanol"},
    {"fr", "French", "Francais"},
    {"de", "German", "Deutsch"},
    {"pt", "Portuguese", "Portugues"},
    {"ru", "Russian", "Russkij"},
    {"zh", "Chinese", "Zhongwen"},
    {"ja", "Japanese", "Nihongo"},
    {"ko", "Korean", "Hangugeo"},
    {"ar", "Arabic", "Arabiyya"},
    {"hi", "Hindi", "Hindi"},
    {"it", "Italian", "Italiano"},
    {"tr", "Turkish", "Turkce"},
    {"pl", "Polish", "Polski"},
    {"nl", "Dutch", "Nederlands"},
    {"sv", "Swedish", "Svenska"},
    {"no", "Norwegian", "Norsk"},
    {"da", "Danish", "Dansk"},
    {"fi", "Finnish", "Suomi"},
    {"th", "Thai", "Thai"},
    {"vi", "Vietnamese", "Tieng Viet"},
    {"id", "Indonesian", "Bahasa Indonesia"},
};

Settings::Settings() {
    load_from_disk();
}

std::string Settings::get_save_path() const {
    return Utils::get_data_dir() + "/settings.json";
}

void Settings::load_from_disk() {
    std::string content = Utils::read_file(get_save_path());
    if (content.empty()) return;

    try {
        auto j = json::parse(content);
        settings_.auto_execute = j.value("auto_execute", false);
        settings_.auto_save = j.value("auto_save", true);
        settings_.font_size = j.value("font_size", 14.0f);
        settings_.top_most = j.value("top_most", false);
        settings_.unlock_fps = j.value("unlock_fps", false);
        settings_.target_fps = j.value("target_fps", 60);
        settings_.save_console = j.value("save_console", true);
        settings_.word_wrap = j.value("word_wrap", true);
        settings_.line_numbers = j.value("line_numbers", true);
        settings_.auto_indent = j.value("auto_indent", true);
        settings_.transparency = j.value("transparency", 1.0f);
        settings_.theme_index = j.value("theme_index", 0);
        settings_.accent_index = j.value("accent_index", 0);
        settings_.background_index = j.value("background_index", 0);
        settings_.language_index = j.value("language_index", 0);
        settings_.max_console_logs = j.value("max_console_logs", 5000);
        settings_.show_fps = j.value("show_fps", true);
        settings_.confirm_execute = j.value("confirm_execute", false);
        settings_.ai_api_key = j.value("ai_api_key", "");

        if (!settings_.ai_api_key.empty()) {
            strncpy(api_key_buf_, settings_.ai_api_key.c_str(), sizeof(api_key_buf_) - 1);
        }

        Theme::set_accent(settings_.accent_index);
        Theme::set_background(settings_.background_index);
        Theme::set_transparency(settings_.transparency);
    } catch (...) {}
}

void Settings::save_to_disk() {
    json j = {
        {"auto_execute", settings_.auto_execute},
        {"auto_save", settings_.auto_save},
        {"font_size", settings_.font_size},
        {"top_most", settings_.top_most},
        {"unlock_fps", settings_.unlock_fps},
        {"target_fps", settings_.target_fps},
        {"save_console", settings_.save_console},
        {"word_wrap", settings_.word_wrap},
        {"line_numbers", settings_.line_numbers},
        {"auto_indent", settings_.auto_indent},
        {"transparency", settings_.transparency},
        {"theme_index", settings_.theme_index},
        {"accent_index", settings_.accent_index},
        {"background_index", settings_.background_index},
        {"language_index", settings_.language_index},
        {"max_console_logs", settings_.max_console_logs},
        {"show_fps", settings_.show_fps},
        {"confirm_execute", settings_.confirm_execute},
        {"ai_api_key", settings_.ai_api_key},
    };
    Utils::write_file(get_save_path(), j.dump(2));
}

void Settings::render(float width, float height, std::function<void()> on_change) {
    ImGui::BeginChild("SettingsScroll", ImVec2(width, height), false);

    if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen)) {
        render_general(width);
    }
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Editor", ImGuiTreeNodeFlags_DefaultOpen)) {
        render_editor_settings(width);
    }
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
        render_appearance(width);
    }
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Language")) {
        render_language(width);
    }
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Advanced")) {
        render_advanced(width);
    }
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("About")) {
        render_about(width);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().accent);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    if (ImGui::Button("Save Settings", ImVec2(width - 16, 32))) {
        save_to_disk();
        if (on_change) on_change();
    }
    ImGui::PopStyleColor(2);

    ImGui::Spacing();
    if (ImGui::Button("Reset to Defaults", ImVec2(width - 16, 28))) {
        settings_ = ExecutorSettings();
        Theme::set_accent(0);
        Theme::set_background(0);
        Theme::set_transparency(1.0f);
        save_to_disk();
        if (on_change) on_change();
    }

    ImGui::EndChild();
}

void Settings::render_general(float width) {
    ImGui::Indent(10);

    ImGui::Checkbox("Auto Execute on Load", &settings_.auto_execute);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Automatically execute scripts when loaded from Script Hub");

    ImGui::Checkbox("Auto Save Scripts", &settings_.auto_save);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Automatically save your work periodically");

    ImGui::Checkbox("Save Console History", &settings_.save_console);
    ImGui::Checkbox("Confirm Before Execute", &settings_.confirm_execute);
    ImGui::Checkbox("Show FPS Counter", &settings_.show_fps);

    ImGui::Spacing();
    ImGui::Checkbox("Unlock FPS", &settings_.unlock_fps);
    if (settings_.unlock_fps) {
        ImGui::SetNextItemWidth(200);
        ImGui::SliderInt("Target FPS", &settings_.target_fps, 30, 360);
    }

    ImGui::Unindent(10);
}

void Settings::render_editor_settings(float width) {
    ImGui::Indent(10);

    ImGui::SetNextItemWidth(200);
    ImGui::SliderFloat("Font Size", &settings_.font_size, 10.0f, 24.0f, "%.0f px");

    ImGui::Checkbox("Word Wrap", &settings_.word_wrap);
    ImGui::Checkbox("Line Numbers", &settings_.line_numbers);
    ImGui::Checkbox("Auto Indent", &settings_.auto_indent);

    ImGui::Unindent(10);
}

void Settings::render_appearance(float width) {
    ImGui::Indent(10);

    // Accent colors
    ImGui::TextColored(Theme::colors().accent, "Accent Color:");
    const auto& accents = Theme::get_accents();
    float btn_size = 28.0f;
    for (int i = 0; i < (int)accents.size(); i++) {
        if (i > 0) ImGui::SameLine();
        bool selected = (settings_.accent_index == i);

        ImGui::PushStyleColor(ImGuiCol_Button, accents[i].color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(accents[i].color.x * 0.8f, accents[i].color.y * 0.8f, accents[i].color.z * 0.8f, 1.0f));

        std::string label = (selected ? "##acc_sel_" : "##acc_") + std::to_string(i);
        if (ImGui::Button(label.c_str(), ImVec2(btn_size, btn_size))) {
            settings_.accent_index = i;
            Theme::set_accent(i);
        }
        ImGui::PopStyleColor(2);

        if (selected) {
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(255, 255, 255, 255), 4.0f, 0, 2.0f);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", accents[i].name.c_str());
        }
    }

    ImGui::Spacing();

    // Background colors
    ImGui::TextColored(Theme::colors().accent, "Background:");
    const auto& bgs = Theme::get_backgrounds();
    for (int i = 0; i < (int)bgs.size(); i++) {
        if (i > 0) ImGui::SameLine();
        bool selected = (settings_.background_index == i);

        ImVec4 display_color = ImVec4(bgs[i].color.x + 0.1f, bgs[i].color.y + 0.1f, bgs[i].color.z + 0.1f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, display_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(display_color.x + 0.05f, display_color.y + 0.05f, display_color.z + 0.05f, 1.0f));

        std::string label = (selected ? "##bg_sel_" : "##bg_") + std::to_string(i);
        if (ImGui::Button(label.c_str(), ImVec2(btn_size, btn_size))) {
            settings_.background_index = i;
            Theme::set_background(i);
        }
        ImGui::PopStyleColor(2);

        if (selected) {
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(255, 255, 255, 255), 4.0f, 0, 2.0f);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", bgs[i].name.c_str());
        }
    }

    ImGui::Spacing();

    // Transparency
    ImGui::TextColored(Theme::colors().accent, "Transparency:");
    ImGui::SetNextItemWidth(width - 40);
    if (ImGui::SliderFloat("##transparency", &settings_.transparency, 0.3f, 1.0f, "%.0f%%")) {
        Theme::set_transparency(settings_.transparency);
    }

    ImGui::Unindent(10);
}

void Settings::render_language(float width) {
    ImGui::Indent(10);

    for (int i = 0; i < (int)languages_.size(); i++) {
        bool selected = (settings_.language_index == i);
        std::string label = languages_[i].name + " (" + languages_[i].native_name + ")";

        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().accent);
        }

        if (ImGui::Selectable((label + "##lang_" + std::to_string(i)).c_str(), selected)) {
            settings_.language_index = i;
        }

        if (selected) {
            ImGui::PopStyleColor();
        }
    }

    ImGui::Unindent(10);
}

void Settings::render_advanced(float width) {
    ImGui::Indent(10);

    ImGui::SetNextItemWidth(200);
    ImGui::InputInt("Max Console Logs", &settings_.max_console_logs, 500, 1000);
    settings_.max_console_logs = std::max(100, std::min(50000, settings_.max_console_logs));

    ImGui::Spacing();

    ImGui::TextColored(Theme::colors().accent, "AI API Key (OpenAI):");
    ImGui::SetNextItemWidth(width - 100);
    ImGuiInputTextFlags flags = show_api_key_ ? 0 : ImGuiInputTextFlags_Password;
    if (ImGui::InputText("##api_key", api_key_buf_, sizeof(api_key_buf_), flags)) {
        settings_.ai_api_key = api_key_buf_;
    }
    ImGui::SameLine();
    if (ImGui::Button(show_api_key_ ? "Hide" : "Show")) {
        show_api_key_ = !show_api_key_;
    }

    ImGui::Spacing();
    ImGui::TextColored(Theme::colors().text_dim, "Data directory: %s", Utils::get_data_dir().c_str());

    ImGui::Unindent(10);
}

void Settings::render_about(float width) {
    ImGui::Indent(10);

    ImGui::TextColored(Theme::colors().accent, "Crynos Executor v2.0");
    ImGui::Spacing();
    ImGui::TextColored(Theme::colors().text, "Advanced Roblox Script Executor");
    ImGui::TextColored(Theme::colors().text_dim, "Built with C++ / Dear ImGui / OpenGL");
    ImGui::Spacing();
    ImGui::TextColored(Theme::colors().text_dim, "Features:");
    ImGui::BulletText("Multi-tab script editor with line numbers");
    ImGui::BulletText("Dual console (Crynos + Roblox) with filtering");
    ImGui::BulletText("Script Hub with 4 search APIs");
    ImGui::BulletText("AI-powered Lua script generation");
    ImGui::BulletText("Customizable themes and accent colors");
    ImGui::BulletText("22 language localization support");
    ImGui::BulletText("FPS counter and performance tools");
    ImGui::BulletText("Script export and import");
    ImGui::Spacing();
    ImGui::TextColored(Theme::colors().text_dim, "Script APIs: ScriptBlox, Rscripts, ScriptSearch, RawScripts");

    ImGui::Unindent(10);
}

ExecutorSettings& Settings::get() { return settings_; }
const ExecutorSettings& Settings::get() const { return settings_; }

const LanguageInfo& Settings::current_language() const {
    return languages_[settings_.language_index];
}

const std::vector<LanguageInfo>& Settings::get_languages() {
    return languages_;
}
