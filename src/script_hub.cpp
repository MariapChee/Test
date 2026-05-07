#include "script_hub.h"
#include "theme.h"
#include "utils.h"
#include <json.hpp>
#include <algorithm>

using json = nlohmann::json;

ScriptHub::ScriptHub(HttpClient& http) : http_(http) {
    apis_ = {
        {"ScriptBlox", "https://scriptblox.com", "/api/script/search?q=", ScriptSource::ScriptBlox,
         ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "SB", true},
        {"Rscripts", "https://rscripts.net", "/api/scripts?q=", ScriptSource::Rscripts,
         ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "RS", true},
        {"ScriptSearch", "https://scriptsearch.org", "/api/search?q=", ScriptSource::ScriptSearch,
         ImVec4(0.3f, 0.9f, 0.5f, 1.0f), "SS", true},
        {"RawScripts", "https://rawscripts.net", "/api/search?q=", ScriptSource::RawScripts,
         ImVec4(0.9f, 0.6f, 0.2f, 1.0f), "RW", true},
    };
}

void ScriptHub::render(float width, float height,
                       std::function<void(const std::string&)> on_paste,
                       std::function<void(const std::string&)> on_execute,
                       std::function<void(const ScriptEntry&)> on_save) {
    render_api_selector(width);
    ImGui::Spacing();
    render_search_bar(width);
    ImGui::Spacing();

    if (selected_script_ >= 0 && selected_script_ < (int)results_.size()) {
        render_script_detail(width, height - 120, on_paste, on_execute, on_save);
    } else {
        render_results(width, height - 120, on_paste, on_execute, on_save);
    }
}

void ScriptHub::render_api_selector(float width) {
    ImGui::TextColored(Theme::colors().accent, "Script Sources:");
    ImGui::SameLine();
    ImGui::TextColored(Theme::colors().text_dim, "(%d APIs)", (int)apis_.size());

    float btn_width = (width - 30) / (float)apis_.size();
    for (int i = 0; i < (int)apis_.size(); i++) {
        if (i > 0) ImGui::SameLine();

        bool is_active = (active_api_ == i) || (active_api_ == -1 && apis_[i].enabled);

        if (is_active) {
            ImGui::PushStyleColor(ImGuiCol_Button, apis_[i].brand_color);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().surface);
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().text_dim);
        }

        std::string label = apis_[i].icon_letter + " " + apis_[i].name + "##api_" + std::to_string(i);
        if (ImGui::Button(label.c_str(), ImVec2(btn_width, 30))) {
            if (active_api_ == i) {
                active_api_ = -1;
            } else {
                active_api_ = i;
            }
        }
        ImGui::PopStyleColor(2);

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", apis_[i].name.c_str());
            ImGui::TextColored(Theme::colors().text_dim, "%s", apis_[i].base_url.c_str());
            ImGui::EndTooltip();
        }
    }

    // All button
    ImGui::SameLine();
    bool all_active = (active_api_ == -1);
    if (all_active) {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().accent);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    }
    if (ImGui::Button("All##api_all", ImVec2(40, 30))) {
        active_api_ = -1;
    }
    if (all_active) {
        ImGui::PopStyleColor(2);
    }
}

void ScriptHub::render_search_bar(float width) {
    ImGui::SetNextItemWidth(width - 80);
    bool enter_pressed = ImGui::InputTextWithHint("##hub_search", "Search scripts across all APIs...",
                                                   search_buf_, sizeof(search_buf_),
                                                   ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();

    if (is_searching_) {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().surface);
        ImGui::Button("...", ImVec2(70, 0));
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().accent);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        if (ImGui::Button("Search", ImVec2(70, 0)) || enter_pressed) {
            search(std::string(search_buf_));
        }
        ImGui::PopStyleColor(2);
    }
}

void ScriptHub::render_results(float width, float height,
                                std::function<void(const std::string&)> on_paste,
                                std::function<void(const std::string&)> on_execute,
                                std::function<void(const ScriptEntry&)> on_save) {
    if (results_.empty() && !is_searching_) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 30);
        float text_w = ImGui::CalcTextSize("Search for scripts using the bar above").x;
        ImGui::SetCursorPosX((width - text_w) / 2);
        ImGui::TextColored(Theme::colors().text_dim, "Search for scripts using the bar above");

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
        text_w = ImGui::CalcTextSize("Powered by ScriptBlox, Rscripts, ScriptSearch & RawScripts").x;
        ImGui::SetCursorPosX((width - text_w) / 2);
        ImGui::TextColored(Theme::colors().text_dim, "Powered by ScriptBlox, Rscripts, ScriptSearch & RawScripts");
        return;
    }

    if (is_searching_) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 30);
        float text_w = ImGui::CalcTextSize("Searching...").x;
        ImGui::SetCursorPosX((width - text_w) / 2);
        ImGui::TextColored(Theme::colors().accent, "Searching...");
        return;
    }

    ImGui::TextColored(Theme::colors().text_dim, "%d scripts found", (int)results_.size());
    ImGui::Separator();

    ImGui::BeginChild("ScriptResults", ImVec2(width, height - 30), false);

    for (int i = 0; i < (int)results_.size(); i++) {
        const auto& script = results_[i];
        ImGui::PushID(i);

        ImVec4 src_color = get_source_color(script.source);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().surface);
        ImGui::BeginChild("ScriptCard", ImVec2(width - 16, 80), true);

        // Source badge
        ImGui::PushStyleColor(ImGuiCol_Text, src_color);
        ImGui::Text("[%s]", get_source_name(script.source));
        ImGui::PopStyleColor();
        ImGui::SameLine();

        // Title
        ImGui::TextColored(Theme::colors().text, "%s", script.title.c_str());

        // Game info
        if (script.is_universal) {
            ImGui::TextColored(Theme::colors().success, "Universal");
        } else if (!script.game_name.empty()) {
            ImGui::TextColored(Theme::colors().text_dim, "Game: %s", script.game_name.c_str());
        }

        // Action buttons
        ImGui::SameLine(width - 220);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 10);

        ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().accent);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        if (ImGui::Button("Execute##exec")) {
            if (on_execute) on_execute(script.script_content);
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine();
        if (ImGui::Button("Paste##paste")) {
            if (on_paste) on_paste(script.script_content);
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().success);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        if (ImGui::Button("Save##save")) {
            if (on_save) on_save(script);
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine();
        if (ImGui::Button("View##view")) {
            selected_script_ = i;
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::PopID();
    }

    ImGui::EndChild();
}

void ScriptHub::render_script_detail(float width, float height,
                                      std::function<void(const std::string&)> on_paste,
                                      std::function<void(const std::string&)> on_execute,
                                      std::function<void(const ScriptEntry&)> on_save) {
    const auto& script = results_[selected_script_];

    if (ImGui::Button("<< Back to results")) {
        selected_script_ = -1;
        return;
    }

    ImGui::SameLine();
    ImGui::TextColored(Theme::colors().accent, "%s", script.title.c_str());

    ImGui::Spacing();

    // Script info
    ImVec4 src_color = get_source_color(script.source);
    ImGui::TextColored(src_color, "Source: %s", get_source_name(script.source));

    if (script.is_universal) {
        ImGui::SameLine();
        ImGui::TextColored(Theme::colors().success, "| Universal");
    }
    if (!script.game_name.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(Theme::colors().text_dim, "| Game: %s", script.game_name.c_str());
    }
    if (script.views > 0) {
        ImGui::SameLine();
        ImGui::TextColored(Theme::colors().text_dim, "| Views: %d", script.views);
    }
    if (script.verified) {
        ImGui::SameLine();
        ImGui::TextColored(Theme::colors().success, "| Verified");
    }

    ImGui::Spacing();

    // Action buttons
    ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().accent);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    if (ImGui::Button("Execute Script", ImVec2(130, 30))) {
        if (on_execute) on_execute(script.script_content);
        selected_script_ = -1;
    }
    ImGui::PopStyleColor(2);

    ImGui::SameLine();
    if (ImGui::Button("Paste to Editor", ImVec2(130, 30))) {
        if (on_paste) on_paste(script.script_content);
        selected_script_ = -1;
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().success);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    if (ImGui::Button("Save Script", ImVec2(130, 30))) {
        if (on_save) on_save(script);
    }
    ImGui::PopStyleColor(2);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Script preview
    ImGui::TextColored(Theme::colors().accent, "Script Preview:");
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().editor_bg);
    ImGui::BeginChild("ScriptPreview", ImVec2(width, height - 120), true);
    ImGui::TextWrapped("%s", script.script_content.c_str());
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void ScriptHub::search(const std::string& query) {
    if (query.empty() || is_searching_) return;

    search_query_ = query;
    is_searching_ = true;
    results_.clear();
    selected_script_ = -1;

    if (active_api_ == -1 || active_api_ == 0) search_scriptblox(query);
    if (active_api_ == -1 || active_api_ == 1) search_rscripts(query);
    if (active_api_ == -1 || active_api_ == 2) search_scriptsearch(query);
    if (active_api_ == -1 || active_api_ == 3) search_rawscripts(query);
}

void ScriptHub::search_scriptblox(const std::string& query) {
    std::string url = apis_[0].base_url + apis_[0].search_endpoint + Utils::url_encode(query);
    http_.get_async(url, [this](HttpResponse response) {
        if (response.success) {
            parse_scriptblox(response.body);
        }
        is_searching_ = false;
    });
}

void ScriptHub::search_rscripts(const std::string& query) {
    std::string url = apis_[1].base_url + apis_[1].search_endpoint + Utils::url_encode(query);
    http_.get_async(url, [this](HttpResponse response) {
        if (response.success) {
            parse_rscripts(response.body);
        }
        is_searching_ = false;
    });
}

void ScriptHub::search_scriptsearch(const std::string& query) {
    std::string url = apis_[2].base_url + apis_[2].search_endpoint + Utils::url_encode(query);
    http_.get_async(url, [this](HttpResponse response) {
        if (response.success) {
            parse_scriptsearch(response.body);
        }
        is_searching_ = false;
    });
}

void ScriptHub::search_rawscripts(const std::string& query) {
    std::string url = apis_[3].base_url + apis_[3].search_endpoint + Utils::url_encode(query);
    http_.get_async(url, [this](HttpResponse response) {
        if (response.success) {
            parse_rawscripts(response.body);
        }
        is_searching_ = false;
    });
}

void ScriptHub::parse_scriptblox(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);
        if (j.contains("result") && j["result"].contains("scripts")) {
            for (const auto& s : j["result"]["scripts"]) {
                ScriptEntry entry;
                entry.title = s.value("title", "Untitled");
                entry.script_content = s.value("script", "");
                entry.is_universal = s.value("isUniversal", false);
                entry.verified = s.value("verified", false);
                entry.views = s.value("views", 0);
                entry.source = ScriptSource::ScriptBlox;
                if (s.contains("game") && s["game"].is_object()) {
                    entry.game_name = s["game"].value("name", "");
                    entry.game_id = s["game"].value("gameId", "");
                }
                if (!entry.script_content.empty()) {
                    results_.push_back(entry);
                }
            }
        }
    } catch (...) {}
}

void ScriptHub::parse_rscripts(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);
        auto scripts_arr = j.is_array() ? j : (j.contains("scripts") ? j["scripts"] : json::array());
        for (const auto& s : scripts_arr) {
            ScriptEntry entry;
            entry.title = s.value("title", s.value("name", "Untitled"));
            entry.script_content = s.value("script", s.value("content", ""));
            entry.game_name = s.value("game", "");
            entry.is_universal = s.value("universal", false);
            entry.verified = s.value("verified", false);
            entry.views = s.value("views", 0);
            entry.source = ScriptSource::Rscripts;
            if (!entry.script_content.empty()) {
                results_.push_back(entry);
            }
        }
    } catch (...) {}
}

void ScriptHub::parse_scriptsearch(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);
        auto scripts_arr = j.is_array() ? j : (j.contains("data") ? j["data"] : json::array());
        for (const auto& s : scripts_arr) {
            ScriptEntry entry;
            entry.title = s.value("title", s.value("name", "Untitled"));
            entry.script_content = s.value("script", s.value("code", ""));
            entry.game_name = s.value("game", s.value("gameName", ""));
            entry.is_universal = s.value("isUniversal", false);
            entry.source = ScriptSource::ScriptSearch;
            if (!entry.script_content.empty()) {
                results_.push_back(entry);
            }
        }
    } catch (...) {}
}

void ScriptHub::parse_rawscripts(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);
        auto scripts_arr = j.is_array() ? j : (j.contains("results") ? j["results"] : json::array());
        for (const auto& s : scripts_arr) {
            ScriptEntry entry;
            entry.title = s.value("title", s.value("name", "Untitled"));
            entry.script_content = s.value("script", s.value("content", ""));
            entry.game_name = s.value("game", "");
            entry.is_universal = s.value("universal", s.value("isUniversal", false));
            entry.source = ScriptSource::RawScripts;
            if (!entry.script_content.empty()) {
                results_.push_back(entry);
            }
        }
    } catch (...) {}
}

ImVec4 ScriptHub::get_source_color(ScriptSource source) const {
    for (const auto& api : apis_) {
        if (api.source == source) return api.brand_color;
    }
    return Theme::colors().text;
}

const char* ScriptHub::get_source_name(ScriptSource source) const {
    switch (source) {
        case ScriptSource::ScriptBlox:    return "ScriptBlox";
        case ScriptSource::Rscripts:      return "Rscripts";
        case ScriptSource::ScriptSearch:  return "ScriptSearch";
        case ScriptSource::RawScripts:    return "RawScripts";
        default: return "Unknown";
    }
}

bool ScriptHub::is_searching() const { return is_searching_; }
int ScriptHub::get_result_count() const { return (int)results_.size(); }
const std::vector<ScriptAPI>& ScriptHub::get_apis() const { return apis_; }

void ScriptHub::toggle_api(int index) {
    if (index >= 0 && index < (int)apis_.size()) {
        apis_[index].enabled = !apis_[index].enabled;
    }
}
