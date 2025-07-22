// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Include the main libnx system header, for Switch development
#include <switch.h>

#include <openshock.hpp>

#define CONFIG_FILE "/config/openshock.txt"
#define TOKEN_LENGTH 64
char token[TOKEN_LENGTH + 1] = { 0 };

// #define MANGO_VERSION

void cleanup();
void main_fail(const char* reason);

bool create_configuration() {
    FILE* config_file = fopen(CONFIG_FILE, "w");

    SwkbdConfig kbd;
    Result ret = swkbdCreate(&kbd, 0);

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

    fread(token, TOKEN_LENGTH, 1, config_file);
    fclose(config_file);

    return true;
}

OpenShock init() {
    bool result = load_configuration();
    if (!result) {
        main_fail("Failed to load configuration");
    }

    OpenShock openshock(token);

    result = openshock.request_shockers();
    if (!result) {
        main_fail("Failed to load shockers");
    }

#ifdef MANGO_VERSION
    printf("With love for the cutie <3\n\n");
#endif

    return openshock;
}



int main(int argc, char* argv[]) {
    consoleInit(NULL);

    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    socketInitializeDefault();

    curl_global_init(CURL_GLOBAL_DEFAULT);

    OpenShock openshock = init();


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
    printf("X = Show log\n");
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


        if (kDown & HidNpadButton_X) {
            for (std::string line : openshock.get_logs()) {
                printf("%s\n", line.c_str());
            }
        }


        if (update) {
            printf("Intensity: %d %%, Duration %d s, Command: %s\n", intensity, duration, commands[current_command]);
        }

        if (kDown & HidNpadButton_A) {
            openshock.send_command(intensity, duration, commands[current_command]);
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
    curl_global_cleanup();

    socketExit();
    // Deinitialize and clean up resources used by the console (important!)
    consoleExit(NULL);
    exit(EXIT_SUCCESS);
}