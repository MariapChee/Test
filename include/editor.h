#pragma once
#include <string>
#include <vector>
#include "imgui.h"

struct EditorTab {
    std::string id;
    std::string name;
    std::string content;
    bool is_saved = true;
    bool is_renaming = false;
    char rename_buf[128] = "";
};

class Editor {
public:
    Editor();

    void render(float width, float height);
    void set_content(const std::string& content);
    std::string get_content() const;
    std::string get_active_tab_name() const;

    void add_tab();
    void close_tab(int index);
    void set_font_size(float size);
    void set_word_wrap(bool wrap);
    void set_line_numbers(bool show);
    void set_auto_indent(bool indent);

    int get_char_count() const;
    int get_line_count() const;
    int get_tab_count() const;

    bool has_unsaved_changes() const;
    void mark_saved();

private:
    std::vector<EditorTab> tabs_;
    int active_tab_ = 0;
    float font_size_ = 14.0f;
    bool word_wrap_ = true;
    bool line_numbers_ = true;
    bool auto_indent_ = true;

    void render_tabs(float width);
    void render_editor_area(float width, float height);
    void render_status_bar(float width);
    std::string generate_id();
};
