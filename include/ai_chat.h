#pragma once
#include <string>
#include <vector>
#include <functional>
#include "imgui.h"
#include "http_client.h"

struct ChatMessage {
    std::string id;
    std::string role; // "user" or "assistant"
    std::string content;
    long long timestamp;
};

class AiChat {
public:
    AiChat(HttpClient& http);

    void render(float width, float height,
                std::function<void(const std::string&)> on_insert_code);

    void send_message(const std::string& message);
    void set_api_key(const std::string& key);
    bool is_generating() const;
    void clear_history();

private:
    HttpClient& http_;
    std::vector<ChatMessage> messages_;
    char input_buf_[2048] = "";
    bool is_generating_ = false;
    std::string api_key_;

    void render_messages(float width, float height);
    void render_input(float width, std::function<void(const std::string&)> on_insert_code);
    std::string extract_code(const std::string& text) const;
    std::string generate_id() const;
};
