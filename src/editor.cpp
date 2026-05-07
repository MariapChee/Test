#include "editor.h"
#include "theme.h"
#include <sstream>
#include <chrono>
#include <algorithm>
#include <cstring>

Editor::Editor() {
    EditorTab tab;
    tab.id = generate_id();
    tab.name = "Script 1";
    tab.content = "-- Crynos Executor v2.0\n-- Write your Lua/Luau scripts here\n\nprint(\"Hello from Crynos!\")\n";
    tab.is_saved = true;
    tabs_.push_back(tab);
}

std::string Editor::generate_id() {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return std::to_string(ms);
}

void Editor::render(float width, float height) {
    render_tabs(width);
    render_editor_area(width, height - 60);
    render_status_bar(width);
}

void Editor::render_tabs(float width) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().header);
    ImGui::BeginChild("EditorTabs", ImVec2(width, 32), false, ImGuiWindowFlags_NoScrollbar);

    float tab_x = 4.0f;
    for (int i = 0; i < (int)tabs_.size(); i++) {
        ImGui::SetCursorPos(ImVec2(tab_x, 2));

        bool is_active = (i == active_tab_);
        std::string label = tabs_[i].name;
        if (!tabs_[i].is_saved) label += " *";

        if (tabs_[i].is_renaming) {
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputText(("##rename_" + tabs_[i].id).c_str(), tabs_[i].rename_buf, sizeof(tabs_[i].rename_buf),
                                 ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                if (strlen(tabs_[i].rename_buf) > 0) {
                    tabs_[i].name = tabs_[i].rename_buf;
                }
                tabs_[i].is_renaming = false;
            }
            if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0)) {
                tabs_[i].is_renaming = false;
            }
            tab_x += 110;
        } else {
            if (is_active) {
                ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().tab_active);
                ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().accent);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().tab_bg);
                ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().text_dim);
            }

            float btn_width = ImGui::CalcTextSize(label.c_str()).x + 30;
            if (ImGui::Button((label + "##tab_" + tabs_[i].id).c_str(), ImVec2(btn_width, 26))) {
                active_tab_ = i;
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                tabs_[i].is_renaming = true;
                strncpy(tabs_[i].rename_buf, tabs_[i].name.c_str(), sizeof(tabs_[i].rename_buf) - 1);
            }

            ImGui::PopStyleColor(2);

            // Close button
            if (tabs_.size() > 1) {
                ImGui::SameLine(0, 0);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().text_dim);
                if (ImGui::Button(("x##close_" + tabs_[i].id).c_str(), ImVec2(18, 26))) {
                    close_tab(i);
                    if (active_tab_ >= (int)tabs_.size()) active_tab_ = (int)tabs_.size() - 1;
                    ImGui::PopStyleColor(2);
                    break;
                }
                ImGui::PopStyleColor(2);
            }

            tab_x += btn_width + (tabs_.size() > 1 ? 20 : 4);
        }
    }

    // Add tab button
    ImGui::SetCursorPos(ImVec2(tab_x, 2));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().accent);
    if (ImGui::Button("+##add_tab", ImVec2(26, 26))) {
        add_tab();
    }
    ImGui::PopStyleColor(2);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void Editor::render_editor_area(float width, float height) {
    if (active_tab_ < 0 || active_tab_ >= (int)tabs_.size()) return;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().editor_bg);
    ImGui::BeginChild("EditorArea", ImVec2(width, height), true);

    // Reserve buffer
    static char editor_buf[65536] = "";
    std::string& content = tabs_[active_tab_].content;

    if (content.size() < sizeof(editor_buf)) {
        strncpy(editor_buf, content.c_str(), sizeof(editor_buf) - 1);
        editor_buf[sizeof(editor_buf) - 1] = '\0';
    }

    // Line numbers
    if (line_numbers_) {
        int line_count = get_line_count();
        ImGui::BeginChild("LineNumbers", ImVec2(40, height - 20), false);
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().text_dim);
        for (int i = 1; i <= std::max(1, line_count); i++) {
            ImGui::Text("%3d", i);
        }
        ImGui::PopStyleColor();
        ImGui::EndChild();
        ImGui::SameLine();
    }

    // Text input
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
    float input_width = line_numbers_ ? width - 60 : width - 16;

    if (ImGui::InputTextMultiline("##editor_input", editor_buf, sizeof(editor_buf),
                                   ImVec2(input_width, height - 20), flags)) {
        content = editor_buf;
        tabs_[active_tab_].is_saved = false;
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void Editor::render_status_bar(float width) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().header);
    ImGui::BeginChild("StatusBar", ImVec2(width, 24), false);
    ImGui::SetCursorPos(ImVec2(8, 3));

    ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().text_dim);
    ImGui::Text("Lines: %d  |  Chars: %d  |  Tab: %s  |  %s",
                get_line_count(), get_char_count(),
                tabs_[active_tab_].name.c_str(),
                tabs_[active_tab_].is_saved ? "Saved" : "Modified");
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void Editor::set_content(const std::string& content) {
    if (active_tab_ >= 0 && active_tab_ < (int)tabs_.size()) {
        tabs_[active_tab_].content = content;
        tabs_[active_tab_].is_saved = false;
    }
}

std::string Editor::get_content() const {
    if (active_tab_ >= 0 && active_tab_ < (int)tabs_.size()) {
        return tabs_[active_tab_].content;
    }
    return "";
}

std::string Editor::get_active_tab_name() const {
    if (active_tab_ >= 0 && active_tab_ < (int)tabs_.size()) {
        return tabs_[active_tab_].name;
    }
    return "";
}

void Editor::add_tab() {
    EditorTab tab;
    tab.id = generate_id();
    tab.name = "Script " + std::to_string(tabs_.size() + 1);
    tab.content = "";
    tab.is_saved = true;
    tabs_.push_back(tab);
    active_tab_ = (int)tabs_.size() - 1;
}

void Editor::close_tab(int index) {
    if (index >= 0 && index < (int)tabs_.size() && tabs_.size() > 1) {
        tabs_.erase(tabs_.begin() + index);
        if (active_tab_ >= (int)tabs_.size()) {
            active_tab_ = (int)tabs_.size() - 1;
        }
    }
}

int Editor::get_char_count() const {
    if (active_tab_ >= 0 && active_tab_ < (int)tabs_.size()) {
        return (int)tabs_[active_tab_].content.size();
    }
    return 0;
}

int Editor::get_line_count() const {
    if (active_tab_ >= 0 && active_tab_ < (int)tabs_.size()) {
        const std::string& c = tabs_[active_tab_].content;
        if (c.empty()) return 1;
        return (int)std::count(c.begin(), c.end(), '\n') + 1;
    }
    return 1;
}

int Editor::get_tab_count() const { return (int)tabs_.size(); }

bool Editor::has_unsaved_changes() const {
    for (const auto& tab : tabs_) {
        if (!tab.is_saved) return true;
    }
    return false;
}

void Editor::mark_saved() {
    if (active_tab_ >= 0 && active_tab_ < (int)tabs_.size()) {
        tabs_[active_tab_].is_saved = true;
    }
}

void Editor::set_font_size(float size) { font_size_ = size; }
void Editor::set_word_wrap(bool wrap) { word_wrap_ = wrap; }
void Editor::set_line_numbers(bool show) { line_numbers_ = show; }
void Editor::set_auto_indent(bool indent) { auto_indent_ = indent; }
