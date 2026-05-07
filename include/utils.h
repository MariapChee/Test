#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <sys/stat.h>

namespace Utils {
    std::string get_timestamp();
    std::string get_home_dir();
    std::string get_data_dir();
    void ensure_dir(const std::string& path);
    std::string read_file(const std::string& path);
    bool write_file(const std::string& path, const std::string& content);
    std::string url_encode(const std::string& value);
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delim);
    std::string to_lower(const std::string& str);
}
