#pragma once
#include <string>
#include <vector>
#include <deque>
#include "imgui.h"
#include "theme.h"

enum class LogType {
    Info,
    Warn,
    Error,
    Success,
    FPS,
    Roblox,
    System,
    ScriptHub,
    AI,
    Settings
};

struct ConsoleLog {
    std::string timestamp;
    std::string message;
    LogType type;
    std::string prefix;
};

enum class ConsoleFilter {
    All,
    Info,
    Warn,
    Error,
    Roblox
};

class Console {
public:
    Console();

    void add_log(const std::string& message, LogType type = LogType::Info, const std::string& prefix = "[Crynos]");
    void clear();
    void render(float width, float height);

    void set_filter(ConsoleFilter filter);
    ConsoleFilter get_filter() const;

    bool is_roblox_connected() const;
    void set_roblox_connected(bool connected);

    int get_log_count() const;
    void set_max_logs(int max);

    static ImVec4 get_log_color(LogType type);

private:
    std::deque<ConsoleLog> logs_;
    ConsoleFilter filter_ = ConsoleFilter::All;
    bool auto_scroll_ = true;
    bool roblox_connected_ = false;
    int max_logs_ = 5000;
    char filter_text_[256] = "";
    bool show_timestamps_ = true;

    bool passes_filter(const ConsoleLog& log) const;
};
