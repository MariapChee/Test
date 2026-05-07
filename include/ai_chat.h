#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include "imgui.h"

struct ChatMessage {
    std::string id;
    std::string role;
    std::string content;
    long long timestamp;
};

struct ScriptTemplate {
    std::string name;
    std::vector<std::string> keywords;
    std::string code;
    std::string description;
};

class CrynosAI {
public:
    CrynosAI();
    std::string generate_response(const std::string& prompt);

private:
    std::vector<ScriptTemplate> templates_;
    void init_templates();
    int match_score(const std::string& input, const std::vector<std::string>& keywords) const;
    std::string to_lower(const std::string& s) const;
    std::string generate_custom(const std::string& prompt) const;
};

class AiChat {
public:
    AiChat();

    void render(float width, float height,
                std::function<void(const std::string&)> on_insert_code);

    void send_message(const std::string& message);
    bool is_generating() const;
    void clear_history();

private:
    CrynosAI ai_engine_;
    std::vector<ChatMessage> messages_;
    char input_buf_[2048] = "";
    bool is_generating_ = false;

    void render_messages(float width, float height);
    void render_input(float width, std::function<void(const std::string&)> on_insert_code);
    std::string extract_code(const std::string& text) const;
    std::string generate_id() const;
};
