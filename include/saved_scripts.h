#pragma once
#include <string>
#include <vector>
#include <functional>
#include "imgui.h"
#include "script_hub.h"

struct SavedScript {
    std::string name;
    std::string code;
    std::string source;
    std::string game_name;
    std::string game_id;
    bool is_universal = false;
    long long saved_at = 0;
};

class SavedScripts {
public:
    SavedScripts();

    void render(float width, float height,
                std::function<void(const std::string&)> on_paste,
                std::function<void(const std::string&)> on_execute);

    void save_script(const std::string& name, const std::string& code, const std::string& source = "Local");
    void save_from_hub(const ScriptEntry& entry);
    void remove_script(int index);
    void export_script(int index);

    int get_count() const;
    void load_from_disk();
    void save_to_disk();

private:
    std::vector<SavedScript> scripts_;
    char search_buf_[256] = "";
    int selected_ = -1;
    bool confirm_delete_ = false;
    int delete_index_ = -1;

    std::string get_save_path() const;
};
