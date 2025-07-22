#pragma once

#include <borealis.hpp>
#include <openshock.hpp>

class ShockersTab : public brls::Box {
private:
    OpenShock openshock;
    std::vector<std::string> commands {"Vibrate", "Shock"};

    std::string commandValue = commands[0];
    int intensityValue = 0;
    int durationValue = 0;

public:
    ShockersTab();


    BRLS_BIND(brls::DetailCell, shockers, "shockers");
    BRLS_BIND(brls::SelectorCell, command, "command");
    BRLS_BIND(brls::SliderCell, intensity, "intensity");
    BRLS_BIND(brls::SliderCell, duration, "duration");
    BRLS_BIND(brls::Button, send, "send");

    static brls::View* create();
};
