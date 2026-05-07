#include "http_client.h"

HttpClient::HttpClient() {
    curl_global_init(CURL_GLOBAL_ALL);
}

HttpClient::~HttpClient() {
    curl_global_cleanup();
}

size_t HttpClient::write_callback(void* contents, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)contents, size * nmemb);
    return size * nmemb;
}

HttpResponse HttpClient::get(const std::string& url) {
    HttpResponse response;
    CURL* curl = curl_easy_init();
    if (!curl) {
        response.error = "Failed to initialize CURL";
        return response;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CrynosExecutor/2.0");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        response.error = curl_easy_strerror(res);
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
        response.success = (response.status_code >= 200 && response.status_code < 300);
    }

    curl_easy_cleanup(curl);
    return response;
}

HttpResponse HttpClient::post(const std::string& url, const std::string& body, const std::string& content_type) {
    HttpResponse response;
    CURL* curl = curl_easy_init();
    if (!curl) {
        response.error = "Failed to initialize CURL";
        return response;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Content-Type: " + content_type).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CrynosExecutor/2.0");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        response.error = curl_easy_strerror(res);
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
        response.success = (response.status_code >= 200 && response.status_code < 300);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

void HttpClient::get_async(const std::string& url, std::function<void(HttpResponse)> callback) {
    std::thread([this, url, callback]() {
        auto response = get(url);
        std::lock_guard<std::mutex> lock(mutex_);
        completed_.push({response, callback});
    }).detach();
}

void HttpClient::post_async(const std::string& url, const std::string& body, std::function<void(HttpResponse)> callback) {
    std::thread([this, url, body, callback]() {
        auto response = post(url, body);
        std::lock_guard<std::mutex> lock(mutex_);
        completed_.push({response, callback});
    }).detach();
}

void HttpClient::process_callbacks() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!completed_.empty()) {
        auto result = std::move(completed_.front());
        completed_.pop();
        if (result.callback) {
            result.callback(std::move(result.response));
        }
    }
}
