#pragma once

#include <borealis.hpp>
#include <openshock.hpp>

class RightOrWrongTab : public brls::Box {
private:
    OpenShock openshock;

    int shockDuration;
    int shockIntensity;
    
    void sendVibrate();
    void sendShock();
    void setProfile(int index);

public:
    RightOrWrongTab();

    BRLS_BIND(brls::DetailCell, shockers, "shockers");
    BRLS_BIND(brls::SelectorCell, profile, "profile");
    BRLS_BIND(brls::Button, right, "right");
    BRLS_BIND(brls::Button, wrong, "wrong");

    static brls::View* create();
};
