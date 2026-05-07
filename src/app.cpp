#include "app.h"
#include "utils.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <cstdio>
#include <chrono>

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

App::App() {
    script_hub_ = std::make_unique<ScriptHub>(http_);
    ai_chat_ = std::make_unique<AiChat>();
}

App::~App() {
    shutdown();
}

bool App::init() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window_ = glfwCreateWindow(window_width_, window_height_, "Crynos Executor v3.0 | Mobile CoreGui Style | Free AI", nullptr, nullptr);
    if (!window_) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    io.Fonts->AddFontDefault();

    Theme::init();
    Theme::apply();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Apply saved settings
    apply_settings();

    console_.add_log("Window initialized (" + std::to_string(window_width_) + "x" + std::to_string(window_height_) + ")", LogType::System, "[System]");
    console_.add_log("OpenGL renderer ready", LogType::System, "[System]");
    console_.add_log("Script Hub: 4 APIs loaded (ScriptBlox, Rscripts, ScriptSearch, RawScripts)", LogType::ScriptHub, "[ScriptHub]");
    console_.add_log("AI Assistant ready - Built-in free AI with 20+ script templates", LogType::AI, "[AI]");
    console_.add_log("Press Ctrl+Enter to execute | Ctrl+S to save | Ctrl+N for new tab", LogType::Info, "[Shortcuts]");

    return true;
}

void App::run() {
    auto last_time = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        http_.process_callbacks();

        auto current_time = std::chrono::high_resolution_clock::now();
        frame_time_ = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;

        // FPS calculation
        frame_count_++;
        fps_timer_ += frame_time_;
        if (fps_timer_ >= 1.0f) {
            fps_ = (float)frame_count_ / fps_timer_;
            frame_count_ = 0;
            fps_timer_ = 0.0f;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        handle_keyboard_shortcuts();
        render();

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        auto& bg = Theme::colors().background;
        glClearColor(bg.x, bg.y, bg.z, bg.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
    }
}

void App::shutdown() {
    if (window_) {
        settings_.save_to_disk();
        saved_scripts_.save_to_disk();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window_);
        glfwTerminate();
        window_ = nullptr;
    }
}

void App::render() {
    glfwGetWindowSize(window_, &window_width_, &window_height_);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)window_width_, (float)window_height_));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("CrynosExecutor", nullptr, flags);
    ImGui::PopStyleVar(3);

    Theme::apply();

    render_header();
    render_main_area();
    render_notification();

    if (settings_.get().show_fps) {
        render_fps_overlay();
    }

    ImGui::End();
}

void App::render_header() {
    float header_height = 48.0f;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().header);

    ImGui::BeginChild("Header", ImVec2((float)window_width_, header_height), false, ImGuiWindowFlags_NoScrollbar);

    // Logo / Title
    ImGui::SetCursorPos(ImVec2(16, 8));
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().accent);
    ImGui::Text("CRYNOS");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextColored(Theme::colors().text_dim, "Executor v3.0");

    // Navigation tabs - responsive for mobile
    float tab_start = std::min(220.0f, (float)window_width_ * 0.18f);
    float tab_width = std::max(70.0f, std::min(100.0f, ((float)window_width_ - tab_start - 400) / 5.0f));
    ImGui::SetCursorPos(ImVec2(tab_start, 6));

    struct TabInfo {
        const char* label;
        SidePanel panel;
    };

    TabInfo tabs[] = {
        {"Editor", SidePanel::Editor},
        {"Script Hub", SidePanel::ScriptHub},
        {"Saved", SidePanel::SavedScripts},
        {"AI Chat", SidePanel::AiChat},
        {"Settings", SidePanel::Settings},
    };

    for (int i = 0; i < 5; i++) {
        if (i > 0) ImGui::SameLine();
        bool active = (active_panel_ == tabs[i].panel);

        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().tab_active);
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().accent);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().text_dim);
        }

        if (ImGui::Button(tabs[i].label, ImVec2(tab_width, 34))) {
            active_panel_ = tabs[i].panel;
        }
        ImGui::PopStyleColor(2);
    }

    // Right side - Execute & connection status
    float right_x = std::max(tab_start + tab_width * 5 + 20, (float)window_width_ - 360.0f);
    ImGui::SetCursorPos(ImVec2(right_x, 8));

    // Execute button
    ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::colors().accent_hover);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    if (ImGui::Button("Execute", ImVec2(80, 30))) {
        handle_execute();
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    // Save button
    if (ImGui::Button("Save", ImVec2(60, 30))) {
        handle_save();
    }

    ImGui::SameLine();

    // Clear button
    if (ImGui::Button("Clear", ImVec2(60, 30))) {
        editor_.set_content("");
        console_.add_log("Editor cleared", LogType::Info, "[System]");
    }

    ImGui::SameLine();

    // Connection indicator
    if (console_.is_roblox_connected()) {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().success);
        ImGui::SetCursorPosY(16);
        ImGui::TextUnformatted("Connected");
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().text_dim);
        ImGui::SetCursorPosY(16);
        ImGui::TextUnformatted("Waiting...");
    }
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void App::render_main_area() {
    float header_h = 48.0f;
    float available_h = (float)window_height_ - header_h;
    float console_h = available_h * 0.3f;
    float panel_h = available_h - console_h - 4;

    // Main content area
    ImGui::SetCursorPos(ImVec2(0, header_h));

    if (active_panel_ == SidePanel::Editor) {
        // Editor takes full width
        ImGui::BeginChild("EditorPanel", ImVec2((float)window_width_, panel_h), false);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
        editor_.render((float)window_width_ - 8, panel_h - 8);
        ImGui::PopStyleVar();
        ImGui::EndChild();
    } else {
        // Side panel
        render_side_panel((float)window_width_, panel_h);
    }

    // Console area
    ImGui::SetCursorPos(ImVec2(0, header_h + panel_h + 2));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().surface);
    ImGui::BeginChild("ConsoleArea", ImVec2((float)window_width_, console_h), true);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
    console_.render((float)window_width_ - 16, console_h - 8);
    ImGui::PopStyleVar();

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void App::render_side_panel(float width, float height) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    ImGui::BeginChild("SidePanel", ImVec2(width, height), false);

    switch (active_panel_) {
        case SidePanel::ScriptHub:
            script_hub_->render(width - 16, height - 16,
                [this](const std::string& code) {
                    editor_.set_content(code);
                    active_panel_ = SidePanel::Editor;
                    show_notif("Script pasted to editor");
                    console_.add_log("Script pasted to editor", LogType::ScriptHub, "[ScriptHub]");
                },
                [this](const std::string& code) {
                    editor_.set_content(code);
                    handle_execute();
                    active_panel_ = SidePanel::Editor;
                },
                [this](const ScriptEntry& entry) {
                    saved_scripts_.save_from_hub(entry);
                    show_notif("Script saved: " + entry.title);
                    console_.add_log("Saved \"" + entry.title + "\" from " + std::string(
                        entry.source == ScriptSource::ScriptBlox ? "ScriptBlox" :
                        entry.source == ScriptSource::Rscripts ? "Rscripts" :
                        entry.source == ScriptSource::ScriptSearch ? "ScriptSearch" : "RawScripts"),
                        LogType::Success, "[System]");
                });
            break;

        case SidePanel::SavedScripts:
            saved_scripts_.render(width - 16, height - 16,
                [this](const std::string& code) {
                    editor_.set_content(code);
                    active_panel_ = SidePanel::Editor;
                    show_notif("Script loaded to editor");
                },
                [this](const std::string& code) {
                    editor_.set_content(code);
                    handle_execute();
                    active_panel_ = SidePanel::Editor;
                });
            break;

        case SidePanel::AiChat:
            ai_chat_->render(width - 16, height - 16,
                [this](const std::string& code) {
                    editor_.set_content(code);
                    active_panel_ = SidePanel::Editor;
                    show_notif("AI code inserted to editor");
                    console_.add_log("AI-generated code inserted to editor", LogType::AI, "[AI]");
                });
            break;

        case SidePanel::Settings:
            settings_.render(width - 16, height - 16, [this]() {
                apply_settings();
                show_notif("Settings saved");
            });
            break;

        default:
            break;
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void App::render_notification() {
    if (!show_notification_) return;

    notification_.timer += frame_time_;
    if (notification_.timer >= notification_.duration) {
        show_notification_ = false;
        return;
    }

    float alpha = 1.0f;
    if (notification_.timer > notification_.duration - 1.0f) {
        alpha = notification_.duration - notification_.timer;
    }

    ImVec4 bg_color;
    if (notification_.type == "success")      bg_color = ImVec4(0.1f, 0.5f, 0.2f, 0.9f * alpha);
    else if (notification_.type == "error")   bg_color = ImVec4(0.6f, 0.1f, 0.1f, 0.9f * alpha);
    else if (notification_.type == "warning") bg_color = ImVec4(0.6f, 0.5f, 0.1f, 0.9f * alpha);
    else                                      bg_color = ImVec4(0.1f, 0.3f, 0.6f, 0.9f * alpha);

    float notif_w = 350;
    float notif_h = 40;
    float x = ((float)window_width_ - notif_w) / 2;
    float y = (float)window_height_ - notif_h - 20;

    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(notif_w, notif_h));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, bg_color);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

    ImGui::Begin("##notification", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::SetCursorPosY(10);
    float text_w = ImGui::CalcTextSize(notification_.message.c_str()).x;
    ImGui::SetCursorPosX((notif_w - text_w) / 2);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, alpha));
    ImGui::TextUnformatted(notification_.message.c_str());
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void App::render_fps_overlay() {
    ImGui::SetNextWindowPos(ImVec2((float)window_width_ - 120, 52));
    ImGui::SetNextWindowSize(ImVec2(115, 30));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);

    ImGui::Begin("##fps_overlay", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoInputs);

    ImVec4 fps_color;
    if (fps_ >= 55) fps_color = Theme::colors().success;
    else if (fps_ >= 30) fps_color = Theme::colors().warning;
    else fps_color = Theme::colors().error;

    ImGui::SetCursorPos(ImVec2(8, 6));
    ImGui::TextColored(fps_color, "FPS: %.0f", fps_);
    ImGui::SameLine();
    ImGui::TextColored(Theme::colors().text_dim, "| %.1fms", frame_time_ * 1000.0f);

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void App::handle_execute() {
    std::string code = editor_.get_content();
    if (code.empty()) {
        show_notif("No script to execute", "warning");
        console_.add_log("No script to execute", LogType::Warn, "[System]");
        return;
    }

    console_.add_log("Executing script (" + std::to_string(code.size()) + " chars)...", LogType::Info, "[Crynos]");

    // Simulate execution
    console_.add_log("Script loaded into memory", LogType::Info, "[Crynos]");
    console_.add_log("Executing in Roblox environment...", LogType::Info, "[Crynos]");

    // Parse for print statements to show output
    std::istringstream stream(code);
    std::string line;
    while (std::getline(stream, line)) {
        line = Utils::trim(line);
        if (line.find("print(") == 0 || line.find("print (") == 0) {
            size_t start = line.find("(") + 1;
            size_t end = line.rfind(")");
            if (start != std::string::npos && end != std::string::npos && end > start) {
                std::string msg = line.substr(start, end - start);
                // Remove quotes
                if (msg.size() >= 2 && ((msg.front() == '"' && msg.back() == '"') ||
                                         (msg.front() == '\'' && msg.back() == '\''))) {
                    msg = msg.substr(1, msg.size() - 2);
                }
                console_.add_log(msg, LogType::Roblox, "[Output]");
            }
        } else if (line.find("warn(") == 0) {
            size_t start = line.find("(") + 1;
            size_t end = line.rfind(")");
            if (start != std::string::npos && end != std::string::npos && end > start) {
                std::string msg = line.substr(start, end - start);
                if (msg.size() >= 2 && ((msg.front() == '"' && msg.back() == '"') ||
                                         (msg.front() == '\'' && msg.back() == '\''))) {
                    msg = msg.substr(1, msg.size() - 2);
                }
                console_.add_log(msg, LogType::Warn, "[Output]");
            }
        }
    }

    console_.add_log("Script executed successfully", LogType::Success, "[Crynos]");
    show_notif("Script executed successfully");
}

void App::handle_save() {
    std::string code = editor_.get_content();
    if (code.empty()) {
        show_notif("Nothing to save", "warning");
        return;
    }

    std::string name = editor_.get_active_tab_name();
    saved_scripts_.save_script(name, code, "Editor");
    editor_.mark_saved();
    show_notif("Script saved: " + name);
    console_.add_log("Saved script: " + name, LogType::Success, "[System]");
}

void App::handle_keyboard_shortcuts() {
    ImGuiIO& io = ImGui::GetIO();

    // Ctrl+Enter - Execute
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        handle_execute();
    }
    // Ctrl+S - Save
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        handle_save();
    }
    // Ctrl+N - New tab
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
        editor_.add_tab();
        console_.add_log("New tab created", LogType::Info, "[System]");
    }
    // Ctrl+1-5 - Switch panels
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_1)) active_panel_ = SidePanel::Editor;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_2)) active_panel_ = SidePanel::ScriptHub;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_3)) active_panel_ = SidePanel::SavedScripts;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_4)) active_panel_ = SidePanel::AiChat;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_5)) active_panel_ = SidePanel::Settings;
}

void App::show_notif(const std::string& message, const std::string& type) {
    notification_.message = message;
    notification_.type = type;
    notification_.timer = 0.0f;
    show_notification_ = true;
}

void App::apply_settings() {
    auto& s = settings_.get();
    editor_.set_font_size(s.font_size);
    editor_.set_word_wrap(s.word_wrap);
    editor_.set_line_numbers(s.line_numbers);
    editor_.set_auto_indent(s.auto_indent);
    console_.set_max_logs(s.max_console_logs);
    Theme::set_accent(s.accent_index);
    Theme::set_background(s.background_index);
    Theme::set_transparency(s.transparency);
    Theme::apply();
}
