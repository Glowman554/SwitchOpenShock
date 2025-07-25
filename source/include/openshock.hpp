#pragma once

#include <vector>
#include <string>

#include <curl/curl.h>
#include <json-c/json.h>

#include <curl_response.hpp>


struct shocker {
    std::string name;
    std::string id;
};

class OpenShock {
private:
    std::vector<struct shocker> shockers{};

    char token_header[128] = { 0 };
    std::string token;

    struct curl_slist* curl_headers();
    CURL* curl_prepare(const char* url, struct curl_slist* headers, struct response_string* response);
    bool process_shockers(struct json_object* response);
    struct json_object* build_commands(int intensity, int duration_seconds, const char* command);

    void set_token_header(std::string token);

public:
    OpenShock();

    bool load_token();

    void set_token(std::string token);
    std::string get_token();

    bool request_shockers();
    bool send_command(int intensity, int duration_seconds, const char* command);

    std::vector<struct shocker> get_shockers();
};

extern OpenShock openshock;