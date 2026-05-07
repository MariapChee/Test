#pragma once
#include <string>
#include <vector>
#include <functional>
#include "imgui.h"
#include "http_client.h"

enum class ScriptSource {
    ScriptBlox,
    Rscripts,
    ScriptSearch,
    RawScripts
};

struct ScriptEntry {
    std::string title;
    std::string script_content;
    std::string game_name;
    std::string game_id;
    bool is_universal = false;
    bool verified = false;
    int views = 0;
    ScriptSource source;
};

struct ScriptAPI {
    std::string name;
    std::string base_url;
    std::string search_endpoint;
    ScriptSource source;
    ImVec4 brand_color;
    std::string icon_letter;
    bool enabled = true;
};

class ScriptHub {
public:
    ScriptHub(HttpClient& http);

    void render(float width, float height,
                std::function<void(const std::string&)> on_paste,
                std::function<void(const std::string&)> on_execute,
                std::function<void(const ScriptEntry&)> on_save);

    void search(const std::string& query);
    bool is_searching() const;
    int get_result_count() const;

    const std::vector<ScriptAPI>& get_apis() const;
    void toggle_api(int index);

private:
    HttpClient& http_;
    std::vector<ScriptAPI> apis_;
    std::vector<ScriptEntry> results_;
    std::string search_query_;
    char search_buf_[256] = "";
    bool is_searching_ = false;
    int selected_script_ = -1;
    int active_api_ = -1; // -1 = search all

    void search_scriptblox(const std::string& query);
    void search_rscripts(const std::string& query);
    void search_scriptsearch(const std::string& query);
    void search_rawscripts(const std::string& query);

    void parse_scriptblox(const std::string& json_str);
    void parse_rscripts(const std::string& json_str);
    void parse_scriptsearch(const std::string& json_str);
    void parse_rawscripts(const std::string& json_str);

    void render_api_selector(float width);
    void render_search_bar(float width);
    void render_results(float width, float height,
                        std::function<void(const std::string&)> on_paste,
                        std::function<void(const std::string&)> on_execute,
                        std::function<void(const ScriptEntry&)> on_save);
    void render_script_detail(float width, float height,
                              std::function<void(const std::string&)> on_paste,
                              std::function<void(const std::string&)> on_execute,
                              std::function<void(const ScriptEntry&)> on_save);

    ImVec4 get_source_color(ScriptSource source) const;
    const char* get_source_name(ScriptSource source) const;
};
