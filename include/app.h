#pragma once
#include <string>
#include <memory>
#include "imgui.h"
#include "http_client.h"
#include "editor.h"
#include "console.h"
#include "script_hub.h"
#include "saved_scripts.h"
#include "settings.h"
#include "ai_chat.h"
#include "theme.h"

struct GLFWwindow;

enum class SidePanel {
    Editor,
    ScriptHub,
    SavedScripts,
    Settings,
    AiChat
};

struct Notification {
    std::string message;
    std::string type; // "success", "error", "warning", "info"
    float timer = 0.0f;
    float duration = 4.0f;
};

class App {
public:
    App();
    ~App();

    bool init();
    void run();
    void shutdown();

private:
    GLFWwindow* window_ = nullptr;
    int window_width_ = 1280;
    int window_height_ = 800;

    HttpClient http_;
    Editor editor_;
    Console console_;
    std::unique_ptr<ScriptHub> script_hub_;
    SavedScripts saved_scripts_;
    Settings settings_;
    std::unique_ptr<AiChat> ai_chat_;

    SidePanel active_panel_ = SidePanel::Editor;
    Notification notification_;

    float fps_ = 0.0f;
    float frame_time_ = 0.0f;
    int frame_count_ = 0;
    float fps_timer_ = 0.0f;

    bool show_notification_ = false;

    void render();
    void render_header();
    void render_main_area();
    void render_side_panel(float width, float height);
    void render_console_area(float width, float height);
    void render_notification();
    void render_fps_overlay();

    void handle_execute();
    void handle_save();
    void handle_keyboard_shortcuts();
    void show_notif(const std::string& message, const std::string& type = "success");
    void apply_settings();
};
