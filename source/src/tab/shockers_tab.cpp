#include "tab/shockers_tab.hpp"
#include "dialog.hpp"
#include <algorithm> 

using namespace brls::literals;  // for _i18n

#define VIBRATE_DURATION 1
#define VIBRATE_INTENSITY 25

int findIndex(std::vector<std::string>& array, std::string goal) {
    for (int i = 0; i < array.size(); i++) {
        if (array[i] == goal) {
            return i;
        }
    }

    return -1;
}


void ShockersTab::sendVibrate() {
    bool success = openshock.send_command(VIBRATE_INTENSITY, VIBRATE_DURATION, "Vibrate");
    if (!success) {
        showFailDialog("Failed to send command");
    }
}


void ShockersTab::sendCommand() {
    bool success = openshock.send_command(intensityValue, durationValue, commandValue.c_str());
    if (!success) {
        showFailDialog("Failed to send command");
    }
}


ShockersTab::ShockersTab() {
    this->inflateFromXMLRes("xml/tabs/shockers.xml");

    bool success = openshock.request_shockers();
    if (!success) {
        showFailDialog("Failed to load shockers");
    }

    shockers->setDetailText(fmt::format("Loaded {} shockers", openshock.get_shockers().size()));
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

void ShockersTab::updateSliderText() {
    intensity->setDetailText(fmt::format("{}%", intensityValue));
    duration->setDetailText(fmt::format("{}s", durationValue));
}

brls::View* ShockersTab::create() {
    return new ShockersTab();
}
