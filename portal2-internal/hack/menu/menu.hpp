#pragma once
#include "ui/imgui_custom.hpp"

namespace hack::menu {
    constexpr float_t kMenuWidth = 560.f;
    constexpr float_t kMenuHeight = 480.f;

    inline bool opened = true;

    void render();
    void toggle();
    void setup();
} // namespace hack::menu
