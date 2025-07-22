#include "tab/right_or_wrong_tab.hpp"
#include "dialog.hpp"

using namespace brls::literals;  // for _i18n

#define VIBRATE_DURATION 1
#define VIBRATE_INTENSITY 25

void RightOrWrongTab::sendVibrate() {
    bool success = openshock.send_command(VIBRATE_INTENSITY, VIBRATE_DURATION, "Vibrate");
    if (!success) {
        showFailDialog("Failed to send command");
    }
}

void RightOrWrongTab::sendShock() {
    bool success = openshock.send_command(shockIntensity, shockDuration, "Shock");
    if (!success) {
        showFailDialog("Failed to send command");
    }
}

void RightOrWrongTab::setProfile(int index) {
    switch (index) {
    case 0:
        shockDuration = 1;
        shockIntensity = 5;
        break;
    case 1:
        shockDuration = 1;
        shockIntensity = 25;
        break;
    case 2:
        shockDuration = 2;
        shockIntensity = 50;
        break;
    case 3:
        shockDuration = 2;
        shockIntensity = 100;
        break;
    }

    brls::Logger::info("Duration: {}s, Intensity: {}%", shockDuration, shockIntensity);
}

RightOrWrongTab::RightOrWrongTab() {
    this->inflateFromXMLRes("xml/tabs/right_or_wrong.xml");

    bool success = openshock.request_shockers();
    if (!success) {
        showFailDialog("Failed to load shockers");
    }

    setProfile(0);

    shockers->setDetailText(fmt::format("Loaded {} shockers", openshock.get_shockers().size()));

    profile->init("Profile", { "Calm", "Balanced", "Aggressive", "Death" }, 0, [](int selected) {}, [this](int selected) {
        setProfile(selected);
    });

    right->registerClickAction([this](...){
        sendVibrate();
        return true;
    });

    wrong->registerClickAction([this](...){
        sendVibrate();
        brls::delay((VIBRATE_DURATION + 1) * 1000, [this](...) {
            sendShock();
        });
        return true;
    });
}

brls::View* RightOrWrongTab::create() {
    return new RightOrWrongTab();
}
