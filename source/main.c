// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Include the main libnx system header, for Switch development
#include <switch.h>

#include <curl/curl.h>
#include <json-c/json.h>

#define CONFIG_FILE "/config/openshock.txt"
#define TOKEN_LENGTH 64
// #define MANGO_VERSION

void cleanup();
void main_fail(const char* reason);

struct response_string {
    char* ptr;
    size_t len;
};

struct shocker {
    const char* name;
    const char* id;
};

struct shocker* global_shocker = NULL;
int global_shocker_len = 0;

char token_header[128] = { 0 };

struct response_string* init_response_string() {
    struct response_string* s = malloc(sizeof(struct response_string));
    if (s == NULL) {
        printf("malloc() failed\n");
        exit(EXIT_FAILURE);
    }

    s->len = 0;
    s->ptr = malloc(1);
    if (s->ptr == NULL) {
        printf("malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';

    return s;
}

void cleanup_response_string(struct response_string* s) {
    free(s->ptr);
    free(s);
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct response_string *s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL) {
        printf("realloc() failed\n");
        return 0;
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}



struct curl_slist* openshock_headers() {
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, token_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    return headers;
}

CURL* openshock_curl(const char* url, struct curl_slist* headers, struct response_string* response) {
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);

    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    if (response) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    }

    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Switch OpenShock app/1.0");

    return curl;
}


bool openshock_process_shockers(struct json_object* response) {
    struct json_object *data_array;
    if (!json_object_object_get_ex(response, "data", &data_array)) {
        printf("Missing 'data' array\n");
        return false;
    }

    size_t data_len = json_object_array_length(data_array);
    for (size_t i = 0; i < data_len; ++i) {
        struct json_object *dataNode = json_object_array_get_idx(data_array, i);

        struct json_object *shockers_array;
        if (!json_object_object_get_ex(dataNode, "shockers", &shockers_array)) {
            printf("Missing 'shockers' array in data node\n");
            return false;
        }

        global_shocker_len = json_object_array_length(shockers_array);

        if (global_shocker) {
            free(global_shocker);
        }

        global_shocker = malloc(sizeof(struct shocker) * global_shocker_len);
        if (global_shocker == NULL) {
            printf("malloc() failed\n");
            exit(EXIT_FAILURE);
        }
        
        for (size_t j = 0; j < global_shocker_len; ++j) {
            struct json_object* shockerNode = json_object_array_get_idx(shockers_array, j);

            struct json_object* name_obj;
            struct json_object* id_obj;

            if (json_object_object_get_ex(shockerNode, "name", &name_obj) &&
                json_object_object_get_ex(shockerNode, "id", &id_obj)) {
                
                global_shocker[j].name = strdup((char *)json_object_get_string(name_obj));
                global_shocker[j].id = strdup((char *)json_object_get_string(id_obj));
            }
        }
    }

    return true;
}

bool openshock_request_shockers() {
    bool result = false;

    struct curl_slist* headers = openshock_headers();
    struct response_string* response = init_response_string();
    CURL* curl = openshock_curl("https://api.openshock.app/1/shockers/own", headers, response);


    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        // printf("Response:\n%s\n", response->ptr);

        struct json_object* parsed_json = json_tokener_parse(response->ptr);
        bool success = openshock_process_shockers(parsed_json);
        if (!success) {
            printf("openshock_process_shockers() failed\n");
        } else {
            printf("Loaded %d shockers\n", global_shocker_len);
            result = true;
        }
        json_object_put(parsed_json);
    }

    curl_slist_free_all(headers);
    cleanup_response_string(response);
    curl_easy_cleanup(curl);

    return result;
}


struct json_object* openshock_build_commands(int intensity, int duration_seconds, const char* command) {
    struct json_object *root = json_object_new_object();
    struct json_object *shocks_array = json_object_new_array();

    for (size_t i = 0; i < global_shocker_len; ++i) {
        struct json_object *shock = json_object_new_object();

        json_object_object_add(shock, "id", json_object_new_string(global_shocker[i].id));
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

bool openshock_send_command(int intensity, int duration_seconds, const char* command) {
    bool result = false;

    struct curl_slist* headers = openshock_headers();
    struct response_string* response = init_response_string();
    CURL* curl = openshock_curl("https://api.openshock.app/2/shockers/control", headers, response);

    json_object* root_json = openshock_build_commands(intensity, duration_seconds, command);

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string_ext(root_json, JSON_C_TO_STRING_PRETTY));


    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        // printf("Response:\n%s\n", response->ptr);
    }

    json_object_put(root_json);
    curl_slist_free_all(headers);
    cleanup_response_string(response);
    curl_easy_cleanup(curl);

    return result;
}

void set_token(const char* token) {
    sprintf(token_header, "Open-Shock-Token: %s", token);
}

bool create_configuration() {
    FILE* config_file = fopen(CONFIG_FILE, "w");

    SwkbdConfig kbd;
    Result ret = swkbdCreate(&kbd, 0);

    char token[TOKEN_LENGTH + 1] = { 0 };

    if (!R_SUCCEEDED(ret)) {
        swkbdClose(&kbd);
        return false;
    }

    swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetOkButtonText(&kbd, "Submit");
    swkbdConfigSetSubText(&kbd, "OpenShock api token");
    swkbdConfigSetGuideText(&kbd, "xxxxxxxx");
    swkbdConfigSetStringLenMax(&kbd, TOKEN_LENGTH);

    ret = swkbdShow(&kbd, token, sizeof(token));
    swkbdClose(&kbd);

    if (!R_SUCCEEDED(ret)) {
        return false;
    }

    fwrite(token, sizeof(token), 1, config_file);
    fclose(config_file);

    set_token(token);

    return true;
}

bool load_configuration() {
    FILE* config_file = fopen(CONFIG_FILE, "r");
    if (!config_file) {
        return create_configuration();
    }

    fseek(config_file, 0, SEEK_END);
    int len = ftell(config_file);
    fseek(config_file, 0, SEEK_SET);

    if (len < TOKEN_LENGTH) {
        fclose(config_file);
        return create_configuration();
    }

    char token[TOKEN_LENGTH + 1] = { 0 };
    fread(token, TOKEN_LENGTH, 1, config_file);
    fclose(config_file);

    set_token(token);

    return true;
}

void init() {
    bool result = load_configuration();
    if (!result) {
        main_fail("Failed to load configuration");
    }

    result = openshock_request_shockers();
    if (!result) {
        main_fail("Failed to load shockers");
    }
    for (int i = 0; i < global_shocker_len; i++) {
        printf("Shocker: '%s' '%s'\n", global_shocker[i].name, global_shocker[i].id);
    }
    printf("\n");

#ifdef MANGO_VERSION
    printf("With love for the cutie <3\n\n");
#endif
}



int main(int argc, char* argv[]) {
    consoleInit(NULL);

    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    socketInitializeDefault();

    curl_global_init(CURL_GLOBAL_DEFAULT);

    init();


    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);


    printf("Usage:\n");
    printf("D-Pad Up = Duration up\n");
    printf("D-Pad Down = Duration down\n");
    printf("D-Pad Right = Intensity up\n");
    printf("D-Pad Left = Intensity down\n");
    printf("B = Change mode\n");
    printf("A = Fire\n");
    printf("Y = Change token\n");
    printf("+ = Close application\n");
    consoleUpdate(NULL);

    int intensity = 50;
    int duration = 1;

    const char* commands[] = {
        "Vibrate",
        "Shock"
    };
    int current_command = 0;

    // Main loop
    while(appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus) {
            break;
        }



        bool update = false;
        if (kDown & HidNpadButton_Left) {
            if (intensity > 1) {
                intensity--;
                update = true;
            }
        }
        
        if (kDown & HidNpadButton_Right) {
            if (intensity < 100) {
                intensity++;
                update = true;
            }
        }

        if (kDown & HidNpadButton_Down) {
            if (duration > 1) {
                duration--;
                update = true;
            }
        }
        
        if (kDown & HidNpadButton_Up) {
            if (duration < 10) {
                duration++;
                update = true;
            }
        }

        if (kDown & HidNpadButton_B) {
            current_command = (current_command + 1) % 2;
            update = true;
        }

        if (kDown & HidNpadButton_Y) {
            bool success = create_configuration();
            if (!success) {
                printf("Failed to set new token\n");
            } else {
                cleanup();
            }
        }


        if (update) {
            printf("Intensity: %d %%, Duration %d s, Command: %s\n", intensity, duration, commands[current_command]);
        }

        if (kDown & HidNpadButton_A) {
            openshock_send_command(intensity, duration, commands[current_command]);
        }

        consoleUpdate(NULL);
    }

    cleanup();
    return 0;
}

void main_fail(const char* reason) {
    // Main loop

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);

    printf("An error occurred: %s\n", reason);
    printf("Press Y to change the token!\n");
    consoleUpdate(NULL);

    while(appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus) {
            break;
        }

        if (kDown & HidNpadButton_Y) {
            bool success = create_configuration();
            if (!success) {
                printf("Failed to set new token\n");
            } else {
                cleanup();
            }
        }

        consoleUpdate(NULL);
    }

    cleanup();
}

void cleanup() {
    if (global_shocker) {
        free(global_shocker);
    }

    curl_global_cleanup();

    socketExit();
    // Deinitialize and clean up resources used by the console (important!)
    consoleExit(NULL);
    exit(EXIT_SUCCESS);
}