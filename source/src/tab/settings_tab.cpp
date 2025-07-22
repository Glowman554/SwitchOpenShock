#include "tab/settings_tab.hpp"

using namespace brls::literals;  // for _i18n

SettingsTab::SettingsTab() {
    this->inflateFromXMLRes("xml/tabs/settings.xml");


    input->init("OpenShock Token", openshock.get_token(), [this](std::string text) {
        openshock.set_token(text);
    }, "-", "OpenShock Token", 64);
}

brls::View* SettingsTab::create() {
    return new SettingsTab();
}
