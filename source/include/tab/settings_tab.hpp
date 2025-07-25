#pragma once

#include <borealis.hpp>
#include <openshock.hpp>

class SettingsTab : public brls::Box {
public:
    SettingsTab();

    BRLS_BIND(brls::InputCell, input, "token");

    static brls::View* create();
};
