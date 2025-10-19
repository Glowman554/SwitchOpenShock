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

struct shared_shockers {
    std::string name;
    std::vector<struct shocker> shockers;
};

class OpenShock {
private:
    char token_header[128] = { 0 };
    std::string token;

    struct curl_slist* curl_headers();
    CURL* curl_prepare(const char* url, struct curl_slist* headers, struct response_string* response);
    
    std::vector<struct shocker> process_shockers(struct json_object* response);
    std::vector<struct shared_shockers> process_shared(struct json_object* response);

    struct json_object* build_commands(int intensity, int duration_seconds, const char* command, std::vector<struct shocker> active);

    void set_token_header(std::string token);

public:
    std::vector<struct shocker> shockers{};
    std::vector<struct shared_shockers> shared{};
    
    OpenShock();

    bool load_token();

    void set_token(std::string token);
    std::string get_token();

    bool request_own_shockers();
    bool request_shared_shockers();
    bool send_command(int intensity, int duration_seconds, const char* command, std::vector<struct shocker> active);
};

extern OpenShock openshock;