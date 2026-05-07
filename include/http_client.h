#pragma once
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <queue>
#include <curl/curl.h>

struct HttpResponse {
    long status_code = 0;
    std::string body;
    bool success = false;
    std::string error;
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpResponse get(const std::string& url);
    HttpResponse post(const std::string& url, const std::string& body, const std::string& content_type = "application/json");

    void get_async(const std::string& url, std::function<void(HttpResponse)> callback);
    void post_async(const std::string& url, const std::string& body, std::function<void(HttpResponse)> callback);

    void process_callbacks();

private:
    static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* data);

    struct AsyncResult {
        HttpResponse response;
        std::function<void(HttpResponse)> callback;
    };

    std::mutex mutex_;
    std::queue<AsyncResult> completed_;
};
