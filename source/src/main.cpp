#include <borealis.hpp>
#include <cstdlib>
#include <string>

#include "tab/settings_tab.hpp"
#include "tab/shockers_tab.hpp"
#include "tab/shared_tab.hpp"
#include "tab/right_or_wrong_tab.hpp"
#include "activity/main_activity.hpp"

using namespace brls::literals; // for _i18n

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "-d") == 0) {
            brls::Logger::setLogLevel(brls::LogLevel::LOG_DEBUG);
        } else if (std::strcmp(argv[i], "-o") == 0) {
            const char* path = (i + 1 < argc) ? argv[++i] : "borealis.log";
            brls::Logger::setLogOutput(std::fopen(path, "w+"));
        } else if (std::strcmp(argv[i], "-v") == 0) {
            brls::Application::enableDebuggingView(true);
        }
    }

    if (!brls::Application::init()) {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    brls::Application::createWindow("demo/title"_i18n);

    brls::Application::getPlatform()->setThemeVariant(brls::ThemeVariant::DARK);

    brls::Application::setGlobalQuit(false);

    brls::Application::registerXMLView("SettingsTab", SettingsTab::create);
    brls::Application::registerXMLView("ShockersTab", ShockersTab::create);
    brls::Application::registerXMLView("SharedTab", SharedTab::create);
    brls::Application::registerXMLView("RightOrWrongTab", RightOrWrongTab::create);

    brls::getStyle().addMetric("about/padding_top_bottom", 50);
    brls::getStyle().addMetric("about/padding_sides", 75);
    brls::getStyle().addMetric("about/description_margin", 50);

    brls::Application::pushActivity(new MainActivity());

    while (brls::Application::mainLoop());

    return EXIT_SUCCESS;
}

#ifdef __WINRT__
#include <borealis/core/main.hpp>
#endif
