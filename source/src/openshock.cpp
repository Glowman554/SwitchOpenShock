#include <openshock.hpp>

#include <borealis.hpp>
#include <format>

#ifdef PLATFORM_DESKTOP
#define CONFIG_FILE "openshock.txt"
#endif

#ifdef PLATFORM_SWITCH
#define CONFIG_FILE "/config/openshock.txt"
#endif


#define TOKEN_LENGTH 64

struct curl_slist* OpenShock::curl_headers() {
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, token_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    return headers;
}

CURL* OpenShock::curl_prepare(const char* url, struct curl_slist* headers, struct response_string* response) {
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);

    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    if (response) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_string_writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    }

    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Switch OpenShock app/1.0");

    return curl;
}

std::vector<struct shocker> OpenShock::process_shockers(struct json_object* response) {
    struct json_object* data_array;
    if (!json_object_object_get_ex(response, "data", &data_array)) {
        brls::Logger::error("Missing 'data' array");
        return {};
    }

    std::vector<struct shocker> result{};

    size_t data_len = json_object_array_length(data_array);
    for (size_t i = 0; i < data_len; ++i) {
        struct json_object *dataNode = json_object_array_get_idx(data_array, i);

        struct json_object *shockers_array;
        if (!json_object_object_get_ex(dataNode, "shockers", &shockers_array)) {
            brls::Logger::error("Missing 'shockers' array in data node");
            return {};
        }

        size_t len = json_object_array_length(shockers_array);
        for (size_t j = 0; j < len; ++j) {
            struct json_object* shockerNode = json_object_array_get_idx(shockers_array, j);

            struct json_object* name_obj;
            struct json_object* id_obj;

            if (json_object_object_get_ex(shockerNode, "name", &name_obj) &&
                json_object_object_get_ex(shockerNode, "id", &id_obj)) {
                    
                result.push_back({
                    .name = json_object_get_string(name_obj),
                    .id = json_object_get_string(id_obj)
                });
            }
        }
    }

    return result;
}

std::vector<struct shared_shockers> OpenShock::process_shared(struct json_object* response) {
    struct json_object* data_array;
    if (!json_object_object_get_ex(response, "data", &data_array)) {
        brls::Logger::error("Missing 'data' array");
        return {};
    }

    std::vector<struct shared_shockers> result{};

    size_t data_len = json_object_array_length(data_array);
    for (size_t i = 0; i < data_len; ++i) {
        struct json_object *dataNode = json_object_array_get_idx(data_array, i);

        struct json_object* devices_array;
        if (!json_object_object_get_ex(dataNode, "devices", &devices_array)) {
            brls::Logger::error("Missing 'devices' array in data node");
            return {};
        }

        size_t devices_len = json_object_array_length(devices_array);
        for (size_t j = 0; j < devices_len; ++j) {
            struct json_object* deviceNode = json_object_array_get_idx(devices_array, j);

            struct json_object* shockers_array;
            if (!json_object_object_get_ex(deviceNode, "shockers", &shockers_array)) {
                brls::Logger::error("Missing 'shockers' array in data node");
                return {};
            }

            std::vector<struct shocker> s{};

            size_t len = json_object_array_length(shockers_array);
            for (size_t k = 0; k < len; ++k) {
                struct json_object* shockerNode = json_object_array_get_idx(shockers_array,k);

                struct json_object* name_obj;
                struct json_object* id_obj;

                if (json_object_object_get_ex(shockerNode, "name", &name_obj) &&
                    json_object_object_get_ex(shockerNode, "id", &id_obj)) {
                        
                    s.push_back({
                        .name = json_object_get_string(name_obj),
                        .id = json_object_get_string(id_obj)
                    });
                }
            }

            struct json_object* name_obj;
            if (json_object_object_get_ex(deviceNode, "name", &name_obj)) {
                result.push_back({
                    .name = json_object_get_string(name_obj),
                    .shockers = s
                });
            }
        }
    }

    return result;
}

struct json_object* OpenShock::build_commands(int intensity, int duration_seconds, const char* command, std::vector<struct shocker> active) {
    struct json_object *root = json_object_new_object();
    struct json_object *shocks_array = json_object_new_array();

    for (struct shocker shocker : active) {
        struct json_object *shock = json_object_new_object();

        json_object_object_add(shock, "id", json_object_new_string(shocker.id.c_str()));
        json_object_object_add(shock, "type", json_object_new_string(command));
        json_object_object_add(shock, "intensity", json_object_new_int(intensity));
        json_object_object_add(shock, "duration", json_object_new_int(duration_seconds * 1000));
        json_object_object_add(shock, "exclusive", json_object_new_boolean(false));

        json_object_array_add(shocks_array, shock);
    }

    json_object_object_add(root, "shocks", shocks_array);
    json_object_object_add(root, "customName", json_object_new_string("SwitchOpenShock"));

    return root;
}

void OpenShock::set_token_header(std::string token) {
    this->token = token;
    sprintf(token_header, "Open-Shock-Token: %s", token.c_str());

    shockers.clear();
}

OpenShock::OpenShock() {
}

bool OpenShock::load_token() {
    FILE* config_file = fopen(CONFIG_FILE, "r");
    if (!config_file) {
        brls::Logger::error("Failed to open {}", CONFIG_FILE);
        set_token_header("");
        return false;
    }

    fseek(config_file, 0, SEEK_END);
    int len = ftell(config_file);
    fseek(config_file, 0, SEEK_SET);

    if (len < TOKEN_LENGTH) {
        brls::Logger::error("Token too short {}", len);
        fclose(config_file);
        set_token_header("");
        return false;
    }

    char token[TOKEN_LENGTH + 1] = { 0 };
    fread(token, TOKEN_LENGTH, 1, config_file);
    fclose(config_file);
    
    set_token_header(token);

    return true;
}

void OpenShock::set_token(std::string token) {
    FILE* config_file = fopen(CONFIG_FILE, "w");
    fwrite(token.c_str(), token.size(), 1, config_file);
    fclose(config_file);

    set_token_header(token);
}

std::string OpenShock::get_token() {
    return token;
}


bool OpenShock::request_own_shockers() {
    if (token.empty() && !load_token()) {
        return false;
    }

    if (shockers.size() > 0) {
        return true;
    }

    bool result = false;

    struct curl_slist* headers = curl_headers();
    struct response_string* response = response_string_init();
    CURL* curl = curl_prepare("https://api.openshock.app/1/shockers/own", headers, response);


    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        brls::Logger::error("curl_easy_perform() failed: {}", curl_easy_strerror(res));
    } else {
        brls::Logger::debug("Response: {}", response->ptr);

        struct json_object* parsed_json = json_tokener_parse(response->ptr);
        std::vector<struct shocker> s = process_shockers(parsed_json);
        if (s.empty()) {
            brls::Logger::error("process_shockers() failed");
        } else {
            brls::Logger::info("Loaded {} shockers", shockers.size());

            for (struct shocker shocker : shockers) {
                brls::Logger::info("Shocker: '{}' '{}'", shocker.name.c_str(), shocker.id.c_str());
            }

            shockers = s;
            result = true;
        }
        json_object_put(parsed_json);
    }

    curl_slist_free_all(headers);
    response_string_cleanup(response);
    curl_easy_cleanup(curl);

    return result;
}

bool OpenShock::request_shared_shockers() {
    if (token.empty() && !load_token()) {
        return false;
    }

    if (shared.size() > 0) {
        return true;
    }

    bool result = false;

    struct curl_slist* headers = curl_headers();
    struct response_string* response = response_string_init();
    CURL* curl = curl_prepare("https://api.openshock.app/1/shockers/shared", headers, response);


    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        brls::Logger::error("curl_easy_perform() failed: {}", curl_easy_strerror(res));
    } else {
        brls::Logger::debug("Response: {}", response->ptr);

        struct json_object* parsed_json = json_tokener_parse(response->ptr);
        std::vector<struct shared_shockers> s = process_shared(parsed_json);
        if (s.empty()) {
            brls::Logger::error("process_shared() failed");
        } else {
            brls::Logger::info("Loaded {} shockers", shockers.size());

            for (struct shared_shockers shared : s) {
                for (struct shocker shocker : shared.shockers) {
                    brls::Logger::info("[{}] Shocker: '{}' '{}'", shared.name.c_str(), shocker.name.c_str(), shocker.id.c_str());
                }
            }

            shared = s;
            result = true;
        }
        json_object_put(parsed_json);
    }

    curl_slist_free_all(headers);
    response_string_cleanup(response);
    curl_easy_cleanup(curl);

    return result;
}

bool OpenShock::send_command(int intensity, int duration_seconds, const char* command, std::vector<struct shocker> active) {
    if (!load_token()) {
        return false;
    }

    bool result = false;

    struct curl_slist* headers = curl_headers();
    struct response_string* response = response_string_init();
    CURL* curl = curl_prepare("https://api.openshock.app/2/shockers/control", headers, response);

    json_object* root_json = build_commands(intensity, duration_seconds, command, active);

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string_ext(root_json, JSON_C_TO_STRING_PRETTY));


    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        brls::Logger::error("curl_easy_perform() failed: {}", curl_easy_strerror(res));
    } else {
        brls::Logger::debug("Response: {}", response->ptr);
        result = true;
    }

    json_object_put(root_json);
    curl_slist_free_all(headers);
    response_string_cleanup(response);
    curl_easy_cleanup(curl);

    return result;
}



OpenShock openshock{};