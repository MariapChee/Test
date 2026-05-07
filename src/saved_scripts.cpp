#include "saved_scripts.h"
#include "theme.h"
#include "utils.h"
#include <json.hpp>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <cstring>

using json = nlohmann::json;

SavedScripts::SavedScripts() {
    load_from_disk();
}

std::string SavedScripts::get_save_path() const {
    return Utils::get_data_dir() + "/saved_scripts.json";
}

void SavedScripts::load_from_disk() {
    std::string content = Utils::read_file(get_save_path());
    if (content.empty()) return;

    try {
        auto j = json::parse(content);
        scripts_.clear();
        for (const auto& s : j) {
            SavedScript script;
            script.name = s.value("name", "Untitled");
            script.code = s.value("code", "");
            script.source = s.value("source", "Local");
            script.game_name = s.value("game_name", "");
            script.game_id = s.value("game_id", "");
            script.is_universal = s.value("is_universal", false);
            script.saved_at = s.value("saved_at", 0LL);
            scripts_.push_back(script);
        }
    } catch (...) {}
}

void SavedScripts::save_to_disk() {
    json j = json::array();
    for (const auto& s : scripts_) {
        j.push_back({
            {"name", s.name},
            {"code", s.code},
            {"source", s.source},
            {"game_name", s.game_name},
            {"game_id", s.game_id},
            {"is_universal", s.is_universal},
            {"saved_at", s.saved_at},
        });
    }
    Utils::write_file(get_save_path(), j.dump(2));
}

void SavedScripts::save_script(const std::string& name, const std::string& code, const std::string& source) {
    SavedScript script;
    script.name = name;
    script.code = code;
    script.source = source;
    script.saved_at = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    scripts_.push_back(script);
    save_to_disk();
}

void SavedScripts::save_from_hub(const ScriptEntry& entry) {
    SavedScript script;
    script.name = entry.title;
    script.code = entry.script_content;
    switch (entry.source) {
        case ScriptSource::ScriptBlox:   script.source = "ScriptBlox"; break;
        case ScriptSource::Rscripts:     script.source = "Rscripts"; break;
        case ScriptSource::ScriptSearch: script.source = "ScriptSearch"; break;
        case ScriptSource::RawScripts:   script.source = "RawScripts"; break;
    }
    script.game_name = entry.game_name;
    script.game_id = entry.game_id;
    script.is_universal = entry.is_universal;
    script.saved_at = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    scripts_.push_back(script);
    save_to_disk();
}

void SavedScripts::remove_script(int index) {
    if (index >= 0 && index < (int)scripts_.size()) {
        scripts_.erase(scripts_.begin() + index);
        save_to_disk();
    }
}

void SavedScripts::export_script(int index) {
    if (index >= 0 && index < (int)scripts_.size()) {
        std::string path = Utils::get_data_dir() + "/exports";
        Utils::ensure_dir(path);
        std::string filename = scripts_[index].name;
        std::replace(filename.begin(), filename.end(), ' ', '_');
        path += "/" + filename + ".lua";
        Utils::write_file(path, scripts_[index].code);
    }
}

void SavedScripts::render(float width, float height,
                           std::function<void(const std::string&)> on_paste,
                           std::function<void(const std::string&)> on_execute) {
    // Search bar
    ImGui::SetNextItemWidth(width - 10);
    ImGui::InputTextWithHint("##saved_search", "Search saved scripts...", search_buf_, sizeof(search_buf_));

    ImGui::Spacing();

    if (scripts_.empty()) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 30);
        float text_w = ImGui::CalcTextSize("No saved scripts yet").x;
        ImGui::SetCursorPosX((width - text_w) / 2);
        ImGui::TextColored(Theme::colors().text_dim, "No saved scripts yet");

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
        text_w = ImGui::CalcTextSize("Save scripts from the editor or Script Hub").x;
        ImGui::SetCursorPosX((width - text_w) / 2);
        ImGui::TextColored(Theme::colors().text_dim, "Save scripts from the editor or Script Hub");
        return;
    }

    ImGui::TextColored(Theme::colors().text_dim, "%d saved scripts", (int)scripts_.size());
    ImGui::Separator();

    // Detail view
    if (selected_ >= 0 && selected_ < (int)scripts_.size()) {
        const auto& script = scripts_[selected_];

        if (ImGui::Button("<< Back")) {
            selected_ = -1;
            return;
        }
        ImGui::SameLine();
        ImGui::TextColored(Theme::colors().accent, "%s", script.name.c_str());

        ImGui::TextColored(Theme::colors().text_dim, "Source: %s | %s | %d chars",
                           script.source.c_str(),
                           script.is_universal ? "Universal" : script.game_name.c_str(),
                           (int)script.code.size());

        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().accent);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        if (ImGui::Button("Execute", ImVec2(100, 28))) {
            if (on_execute) on_execute(script.code);
            selected_ = -1;
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine();
        if (ImGui::Button("Paste", ImVec2(100, 28))) {
            if (on_paste) on_paste(script.code);
            selected_ = -1;
        }

        ImGui::SameLine();
        if (ImGui::Button("Export .lua", ImVec2(100, 28))) {
            export_script(selected_);
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().error);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
        if (ImGui::Button("Delete", ImVec2(100, 28))) {
            confirm_delete_ = true;
            delete_index_ = selected_;
        }
        ImGui::PopStyleColor(2);

        if (confirm_delete_ && delete_index_ == selected_) {
            ImGui::TextColored(Theme::colors().warning, "Are you sure?");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().error);
            if (ImGui::Button("Yes, Delete")) {
                remove_script(delete_index_);
                selected_ = -1;
                confirm_delete_ = false;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                confirm_delete_ = false;
            }
        }

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().editor_bg);
        ImGui::BeginChild("SavedScriptPreview", ImVec2(width, height - 180), true);
        ImGui::TextWrapped("%s", script.code.c_str());
        ImGui::EndChild();
        ImGui::PopStyleColor();

        return;
    }

    // Script list
    ImGui::BeginChild("SavedScriptList", ImVec2(width, height - 80), false);

    std::string filter = Utils::to_lower(std::string(search_buf_));
    for (int i = 0; i < (int)scripts_.size(); i++) {
        if (!filter.empty()) {
            std::string lower_name = Utils::to_lower(scripts_[i].name);
            if (lower_name.find(filter) == std::string::npos) continue;
        }

        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().surface);
        ImGui::BeginChild("SavedCard", ImVec2(width - 16, 65), true);

        ImGui::TextColored(Theme::colors().text, "%s", scripts_[i].name.c_str());
        ImGui::TextColored(Theme::colors().text_dim, "%s | %s | %d chars",
                           scripts_[i].source.c_str(),
                           scripts_[i].is_universal ? "Universal" : scripts_[i].game_name.c_str(),
                           (int)scripts_[i].code.size());

        ImGui::SameLine(width - 180);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 12);

        ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().accent);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        if (ImGui::Button("Execute##se")) {
            if (on_execute) on_execute(scripts_[i].code);
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine();
        if (ImGui::Button("Paste##sp")) {
            if (on_paste) on_paste(scripts_[i].code);
        }

        ImGui::SameLine();
        if (ImGui::Button("View##sv")) {
            selected_ = i;
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::PopID();
    }

    ImGui::EndChild();
}

int SavedScripts::get_count() const { return (int)scripts_.size(); }
