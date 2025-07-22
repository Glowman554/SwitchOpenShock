#include "tab/shockers_tab.hpp"

using namespace brls::literals;  // for _i18n

void showFailDialog(std::string message) {
    auto dialog = new brls::Dialog(message);
    dialog->addButton("OK", []() {});
    dialog->open();
}


ShockersTab::ShockersTab() {
    this->inflateFromXMLRes("xml/tabs/shockers.xml");

    bool success = openshock.request_shockers();
    if (!success) {
        showFailDialog("Failed to load shockers");
    }

    shockers->setDetailText(fmt::format("Loaded {} shockers", openshock.get_shockers().size()));

    command->init("Command", commands, 0, [](int selected) {}, [this](int selected) {
        commandValue = commands[selected];
    });

    intensity->init("Intensity", 0, [this](float value){
        intensityValue = (int) (100. * value);
        intensity->setDetailText(fmt::format("{}", intensityValue));
    });

    duration->init("Duration", 0, [this](float value){
        durationValue = (int) (30. * value);
        duration->setDetailText(fmt::format("{}", durationValue));
    });

    send->registerClickAction([this](...){
        bool success = openshock.send_command(intensityValue, durationValue, commandValue.c_str());
        if (!success) {
            showFailDialog("Failed to send command");
        }
        return true;
    });
}

brls::View* ShockersTab::create() {
    return new ShockersTab();
}
