#include "tab/shared_tab.hpp"
#include "dialog.hpp"
#include "utils.hpp"
#include <algorithm> 

using namespace brls::literals;  // for _i18n

#define VIBRATE_DURATION 1
#define VIBRATE_INTENSITY 25

void SharedTab::sendVibrate() {
    if (shareValueIndex < openshock.shared.size()) {
        bool success = openshock.send_command(VIBRATE_INTENSITY, VIBRATE_DURATION, "Vibrate", openshock.shared[shareValueIndex].shockers);
        if (!success) {
            showFailDialog("Failed to send command");
        }
    }
}


void SharedTab::sendCommand() {
    if (shareValueIndex < openshock.shared.size()) {
        bool success = openshock.send_command(intensityValue, durationValue, commandValue.c_str(), openshock.shared[shareValueIndex].shockers);
        if (!success) {
            showFailDialog("Failed to send command");
        }
    }
}


SharedTab::SharedTab() {
    this->inflateFromXMLRes("xml/tabs/shared.xml");

    bool success = openshock.request_shared_shockers();
    if (!success) {
        showFailDialog("Failed to load shockers");
    }

    for (struct shared_shockers s : openshock.shared) {
        shares.push_back(s.name);
    }

    share->init("Share", shares, 0, [](int selected) {}, [this](int selected) {
        shareValueIndex = selected;
        updateShockersText();
    });
    updateShockersText();

    updateSliderText();

    command->init("Command", commands, findIndex(commands, commandValue), [](int selected) {}, [this](int selected) {
        commandValue = commands[selected];
    });

    intensity->init("Intensity", intensityValue / 100., [this](float value){
        intensityValue = (int) (100. * value);
        updateSliderText();
    });

    duration->init("Duration", durationValue / 30., [this](float value){
        durationValue = (int) (30. * value);
        updateSliderText();
    });

    warning->init("Warning", sendWarning, [this](bool on) {
        sendWarning = on;
    });

    send->registerClickAction([this](...){
        if (sendWarning) {
            sendVibrate();
            brls::delay((VIBRATE_DURATION + 1) * 1000, [this](...) {
                sendCommand();
            });
        } else {
            sendCommand();
        }

        return true;
    });
}

void SharedTab::updateSliderText() {
    intensity->setDetailText(fmt::format("{}%", intensityValue));
    duration->setDetailText(fmt::format("{}s", durationValue));
}

void SharedTab::updateShockersText() {
    if (shareValueIndex < openshock.shared.size()) {
        shockers->setDetailText(fmt::format("Loaded {} shockers", openshock.shared[shareValueIndex].shockers.size()));
    }
}

brls::View* SharedTab::create() {
    return new SharedTab();
}
