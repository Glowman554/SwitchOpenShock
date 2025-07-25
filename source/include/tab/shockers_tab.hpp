#pragma once

#include <borealis.hpp>
#include <openshock.hpp>

class ShockersTab : public brls::Box {
private:
    std::vector<std::string> commands {"Vibrate", "Shock"};

    std::string commandValue = "Shock";
    int intensityValue = 25;
    int durationValue = 5;
    bool sendWarning = false;
    
    void updateSliderText();
    void sendVibrate();
    void sendCommand();

public:
    ShockersTab();

    BRLS_BIND(brls::DetailCell, shockers, "shockers");
    BRLS_BIND(brls::SelectorCell, command, "command");
    BRLS_BIND(brls::SliderCell, intensity, "intensity");
    BRLS_BIND(brls::SliderCell, duration, "duration");
    BRLS_BIND(brls::BooleanCell, warning, "warning");
    BRLS_BIND(brls::Button, send, "send");

    static brls::View* create();
};
