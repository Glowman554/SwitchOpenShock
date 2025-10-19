#pragma once

#include <borealis.hpp>
#include <openshock.hpp>

class SharedTab : public brls::Box {
private:
    std::vector<std::string> commands {"Vibrate", "Shock"};
    std::vector<std::string> shares {};

    int shareValueIndex = 0;
    std::string commandValue = "Shock";
    int intensityValue = 25;
    int durationValue = 5;
    bool sendWarning = false;
    
    void updateSliderText();
    void updateShockersText();
    void sendVibrate();
    void sendCommand();

public:
    SharedTab();

    BRLS_BIND(brls::DetailCell, shockers, "shockers");
    BRLS_BIND(brls::SelectorCell, share, "share");
    BRLS_BIND(brls::SelectorCell, command, "command");
    BRLS_BIND(brls::SliderCell, intensity, "intensity");
    BRLS_BIND(brls::SliderCell, duration, "duration");
    BRLS_BIND(brls::BooleanCell, warning, "warning");
    BRLS_BIND(brls::Button, send, "send");

    static brls::View* create();
};
