#include <openshock.hpp>

#include <format>

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

bool OpenShock::process_shockers(struct json_object* response) {
    struct json_object* data_array;
    if (!json_object_object_get_ex(response, "data", &data_array)) {
        logs.push_back("Missing 'data' array");
        return false;
    }

    size_t data_len = json_object_array_length(data_array);
    for (size_t i = 0; i < data_len; ++i) {
        struct json_object *dataNode = json_object_array_get_idx(data_array, i);

        struct json_object *shockers_array;
        if (!json_object_object_get_ex(dataNode, "shockers", &shockers_array)) {
            logs.push_back("Missing 'shockers' array in data node");
            return false;
        }

        size_t len = json_object_array_length(shockers_array);

        shockers.clear();
        for (size_t j = 0; j < len; ++j) {
            struct json_object* shockerNode = json_object_array_get_idx(shockers_array, j);

            struct json_object* name_obj;
            struct json_object* id_obj;

            if (json_object_object_get_ex(shockerNode, "name", &name_obj) &&
                json_object_object_get_ex(shockerNode, "id", &id_obj)) {
                    
                shockers.push_back({
                    .name = json_object_get_string(name_obj),
                    .id = json_object_get_string(id_obj)
                });
            }
        }
    }

    return true;
}

struct json_object* OpenShock::build_commands(int intensity, int duration_seconds, const char* command) {
    struct json_object *root = json_object_new_object();
    struct json_object *shocks_array = json_object_new_array();

    for (struct shocker shocker : shockers) {
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

OpenShock::OpenShock(std::string token) {
    sprintf(token_header, "Open-Shock-Token: %s", token.c_str());
}

bool OpenShock::request_shockers() {
    bool result = false;

    struct curl_slist* headers = curl_headers();
    struct response_string* response = response_string_init();
    CURL* curl = curl_prepare("https://api.openshock.app/1/shockers/own", headers, response);


    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        logs.push_back(std::format("curl_easy_perform() failed: {}", curl_easy_strerror(res)));
    } else {
        logs.push_back(std::format("Response: {}", response->ptr));

        struct json_object* parsed_json = json_tokener_parse(response->ptr);
        bool success = process_shockers(parsed_json);
        if (!success) {
            logs.push_back("openshock_process_shockers() failed");
        } else {
            logs.push_back(std::format("Loaded {} shockers", shockers.size()));

            for (struct shocker shocker : shockers) {
                logs.push_back(std::format("Shocker: '{}' '{}'", shocker.name.c_str(), shocker.id.c_str()));
            }

            result = true;
        }
        json_object_put(parsed_json);
    }

    curl_slist_free_all(headers);
    response_string_cleanup(response);
    curl_easy_cleanup(curl);

    return result;
}

bool OpenShock::send_command(int intensity, int duration_seconds, const char* command) {
    bool result = false;

    struct curl_slist* headers = curl_headers();
    struct response_string* response = response_string_init();
    CURL* curl = curl_prepare("https://api.openshock.app/2/shockers/control", headers, response);

    json_object* root_json = build_commands(intensity, duration_seconds, command);

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string_ext(root_json, JSON_C_TO_STRING_PRETTY));


    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        logs.push_back(std::format("curl_easy_perform() failed: {}", curl_easy_strerror(res)));
    } else {
        logs.push_back(std::format("Response: {}", response->ptr));
    }

    json_object_put(root_json);
    curl_slist_free_all(headers);
    response_string_cleanup(response);
    curl_easy_cleanup(curl);

    return result;
}

std::vector<struct shocker> OpenShock::get_shockers() {
    return shockers;
}

std::vector<std::string> OpenShock::get_logs() {
    return logs;
}