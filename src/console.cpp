#include "console.h"
#include "utils.h"
#include <algorithm>

Console::Console() {
    add_log("Executor initialized successfully v2.0", LogType::Success, "[Crynos]");
    add_log("Ready to execute scripts", LogType::System, "[System]");
    add_log("Roblox console will show real game output when connected", LogType::Info, "[System]");
    add_log("Type your Lua/Luau scripts in the editor and click Execute", LogType::Info, "[System]");
}

void Console::add_log(const std::string& message, LogType type, const std::string& prefix) {
    ConsoleLog log;
    log.timestamp = Utils::get_timestamp();
    log.message = message;
    log.type = type;
    log.prefix = prefix;

    logs_.push_back(log);
    if ((int)logs_.size() > max_logs_) {
        logs_.pop_front();
    }
}

void Console::clear() {
    logs_.clear();
    add_log("Console cleared", LogType::Info, "[System]");
}

bool Console::passes_filter(const ConsoleLog& log) const {
    switch (filter_) {
        case ConsoleFilter::Info:
            return log.type == LogType::Info || log.type == LogType::System || log.type == LogType::Success;
        case ConsoleFilter::Warn:
            return log.type == LogType::Warn;
        case ConsoleFilter::Error:
            return log.type == LogType::Error;
        case ConsoleFilter::Roblox:
            return log.type == LogType::Roblox;
        default:
            break;
    }

    if (strlen(filter_text_) > 0) {
        std::string lower_msg = Utils::to_lower(log.message);
        std::string lower_filter = Utils::to_lower(std::string(filter_text_));
        return lower_msg.find(lower_filter) != std::string::npos;
    }

    return true;
}

ImVec4 Console::get_log_color(LogType type) {
    switch (type) {
        case LogType::Info:      return Theme::colors().text;
        case LogType::Warn:      return Theme::colors().warning;
        case LogType::Error:     return Theme::colors().error;
        case LogType::Success:   return Theme::colors().success;
        case LogType::FPS:       return Theme::colors().accent;
        case LogType::Roblox:    return ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
        case LogType::System:    return Theme::colors().text_dim;
        case LogType::ScriptHub: return ImVec4(0.8f, 0.6f, 1.0f, 1.0f);
        case LogType::AI:        return ImVec4(1.0f, 0.8f, 0.4f, 1.0f);
        case LogType::Settings:  return Theme::colors().text_dim;
        default:                 return Theme::colors().text;
    }
}

void Console::render(float width, float height) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().console_bg);
    ImGui::BeginChild("ConsoleOutput", ImVec2(width, height - 70), true);

    for (const auto& log : logs_) {
        if (!passes_filter(log)) continue;

        ImVec4 color = get_log_color(log.type);

        if (show_timestamps_) {
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().text_dim);
            ImGui::TextUnformatted(("[" + log.timestamp + "] ").c_str());
            ImGui::PopStyleColor();
            ImGui::SameLine();
        }

        ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().accent);
        ImGui::TextUnformatted((log.prefix + " ").c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextWrapped("%s", log.message.c_str());
        ImGui::PopStyleColor();
    }

    if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    // Filter bar
    ImGui::Separator();
    float button_width = 60.0f;
    const char* filters[] = {"All", "Info", "Warn", "Error", "Roblox"};
    ConsoleFilter filter_vals[] = {ConsoleFilter::All, ConsoleFilter::Info, ConsoleFilter::Warn, ConsoleFilter::Error, ConsoleFilter::Roblox};

    for (int i = 0; i < 5; i++) {
        if (i > 0) ImGui::SameLine();
        bool selected = (filter_ == filter_vals[i]);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().accent);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        }
        if (ImGui::Button(filters[i], ImVec2(button_width, 0))) {
            filter_ = filter_vals[i];
        }
        if (selected) {
            ImGui::PopStyleColor(2);
        }
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    ImGui::InputTextWithHint("##console_filter", "Search logs...", filter_text_, sizeof(filter_text_));

    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        clear();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &auto_scroll_);

    ImGui::SameLine();
    ImGui::Checkbox("Timestamps", &show_timestamps_);

    // Connection status
    ImGui::SameLine();
    ImGui::SetCursorPosX(width - 140);
    if (roblox_connected_) {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().success);
        ImGui::TextUnformatted("Connected");
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().text_dim);
        ImGui::TextUnformatted("Disconnected");
    }
    ImGui::PopStyleColor();
}

void Console::set_filter(ConsoleFilter filter) { filter_ = filter; }
ConsoleFilter Console::get_filter() const { return filter_; }
bool Console::is_roblox_connected() const { return roblox_connected_; }
void Console::set_roblox_connected(bool connected) { roblox_connected_ = connected; }
int Console::get_log_count() const { return (int)logs_.size(); }
void Console::set_max_logs(int max) { max_logs_ = max; }
