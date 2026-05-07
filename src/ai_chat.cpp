#include "ai_chat.h"
#include "theme.h"
#include "utils.h"
#include <json.hpp>
#include <chrono>
#include <cstring>

using json = nlohmann::json;

AiChat::AiChat(HttpClient& http) : http_(http) {
    ChatMessage welcome;
    welcome.id = generate_id();
    welcome.role = "assistant";
    welcome.content = "Welcome to Crynos AI! I can generate Lua/Luau scripts for Roblox.\n\nTry asking me:\n- \"Make an auto farm script\"\n- \"Create an ESP wallhack\"\n- \"Write a speed boost script\"\n- \"Generate a teleport GUI\"";
    welcome.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    messages_.push_back(welcome);
}

std::string AiChat::generate_id() const {
    auto now = std::chrono::steady_clock::now();
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count());
}

void AiChat::render(float width, float height,
                     std::function<void(const std::string&)> on_insert_code) {
    // Header
    ImGui::TextColored(Theme::colors().accent, "Crynos AI Assistant");
    ImGui::SameLine();
    ImGui::TextColored(Theme::colors().text_dim, "- Lua/Luau Script Generator");
    ImGui::SameLine(width - 80);
    if (ImGui::Button("Clear Chat")) {
        clear_history();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    render_messages(width, height - 80);
    render_input(width, on_insert_code);
}

void AiChat::render_messages(float width, float height) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().console_bg);
    ImGui::BeginChild("ChatMessages", ImVec2(width, height), true);

    for (size_t i = 0; i < messages_.size(); i++) {
        const auto& msg = messages_[i];
        ImGui::PushID((int)i);

        bool is_user = (msg.role == "user");

        if (is_user) {
            // User message - right aligned look
            ImGui::PushStyleColor(ImGuiCol_ChildBg,
                ImVec4(Theme::colors().accent.x * 0.15f,
                       Theme::colors().accent.y * 0.15f,
                       Theme::colors().accent.z * 0.15f, 0.8f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().surface);
        }

        float msg_height = ImGui::CalcTextSize(msg.content.c_str(), nullptr, false, width - 40).y + 30;
        msg_height = std::max(msg_height, 40.0f);

        ImGui::BeginChild(("msg_" + msg.id).c_str(), ImVec2(width - 16, msg_height), true);

        // Role label
        if (is_user) {
            ImGui::TextColored(Theme::colors().accent, "You");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Crynos AI");
        }
        ImGui::SameLine();
        ImGui::TextColored(Theme::colors().text_dim, "%s", Utils::get_timestamp().c_str());

        // Content
        ImGui::TextWrapped("%s", msg.content.c_str());

        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PopID();
    }

    if (is_generating_) {
        ImGui::TextColored(Theme::colors().accent, "Generating...");
    }

    // Auto scroll
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void AiChat::render_input(float width, std::function<void(const std::string&)> on_insert_code) {
    ImGui::Spacing();

    float input_width = width - 160;
    ImGui::SetNextItemWidth(input_width);
    bool enter = ImGui::InputTextWithHint("##ai_input",
        "Ask Crynos AI to generate a Lua script...",
        input_buf_, sizeof(input_buf_),
        ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SameLine();

    bool can_send = strlen(input_buf_) > 0 && !is_generating_;

    if (can_send) {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().accent);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    }

    if ((ImGui::Button("Send", ImVec2(70, 0)) || enter) && can_send) {
        send_message(std::string(input_buf_));
        input_buf_[0] = '\0';
    }

    if (can_send) {
        ImGui::PopStyleColor(2);
    }

    ImGui::SameLine();

    // Insert last AI code button
    if (!messages_.empty()) {
        bool has_code = false;
        std::string last_code;
        for (auto it = messages_.rbegin(); it != messages_.rend(); ++it) {
            if (it->role == "assistant" && it != messages_.rbegin()) {
                last_code = extract_code(it->content);
                if (!last_code.empty()) {
                    has_code = true;
                    break;
                }
                // If no code block markers, use the whole content
                last_code = it->content;
                has_code = true;
                break;
            }
        }

        if (has_code && on_insert_code) {
            ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().success);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
            if (ImGui::Button("Insert", ImVec2(70, 0))) {
                on_insert_code(last_code);
            }
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Insert last AI-generated code into editor");
            }
        }
    }
}

void AiChat::send_message(const std::string& message) {
    if (message.empty() || is_generating_) return;

    ChatMessage user_msg;
    user_msg.id = generate_id();
    user_msg.role = "user";
    user_msg.content = message;
    user_msg.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    messages_.push_back(user_msg);

    is_generating_ = true;

    if (api_key_.empty()) {
        // No API key - provide a helpful response
        ChatMessage ai_msg;
        ai_msg.id = generate_id();
        ai_msg.role = "assistant";
        ai_msg.content = "-- No API key configured\n-- Go to Settings > Advanced to add your OpenAI API key\n-- Once configured, I'll generate real Lua scripts for you!\n\n-- Here's a sample for your request:\nprint(\"Crynos Executor v2.0\")\nprint(\"Configure your AI API key in Settings to unlock AI script generation\")";
        ai_msg.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        messages_.push_back(ai_msg);
        is_generating_ = false;
        return;
    }

    json request_body = {
        {"model", "gpt-4o-mini"},
        {"messages", json::array({
            {{"role", "system"}, {"content", "You are a Lua/Luau script generator for Roblox executors. You ONLY generate Lua or Luau code. Do NOT write explanations outside of code comments. Use proper Roblox API syntax. Include helpful comments."}},
            {{"role", "user"}, {"content", message}}
        })},
        {"max_tokens", 2048},
        {"temperature", 0.7}
    };

    std::string url = "https://api.openai.com/v1/chat/completions";
    std::string body = request_body.dump();

    // Custom post with auth header
    http_.post_async(url, body, [this](HttpResponse response) {
        ChatMessage ai_msg;
        ai_msg.id = generate_id();
        ai_msg.role = "assistant";
        ai_msg.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        if (response.success) {
            try {
                auto j = json::parse(response.body);
                if (j.contains("choices") && !j["choices"].empty()) {
                    ai_msg.content = j["choices"][0]["message"]["content"].get<std::string>();
                } else {
                    ai_msg.content = "-- Error: Unexpected API response format";
                }
            } catch (...) {
                ai_msg.content = "-- Error parsing API response";
            }
        } else {
            ai_msg.content = "-- API Error: " + response.error + "\n-- Check your API key in Settings > Advanced";
        }

        messages_.push_back(ai_msg);
        is_generating_ = false;
    });
}

std::string AiChat::extract_code(const std::string& text) const {
    // Look for ```lua ... ``` blocks
    size_t start = text.find("```lua");
    if (start == std::string::npos) start = text.find("```");
    if (start == std::string::npos) return "";

    start = text.find('\n', start);
    if (start == std::string::npos) return "";
    start++;

    size_t end = text.find("```", start);
    if (end == std::string::npos) return "";

    return text.substr(start, end - start);
}

void AiChat::set_api_key(const std::string& key) {
    api_key_ = key;
}

bool AiChat::is_generating() const {
    return is_generating_;
}

void AiChat::clear_history() {
    messages_.clear();
    ChatMessage welcome;
    welcome.id = generate_id();
    welcome.role = "assistant";
    welcome.content = "Chat cleared. Ask me to generate Lua/Luau scripts!";
    welcome.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    messages_.push_back(welcome);
}
