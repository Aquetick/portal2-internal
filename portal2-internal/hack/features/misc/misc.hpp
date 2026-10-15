#pragma once
#include "sdk/portal.hpp"
#include <optional>

namespace hack::features::misc {
    // callbacks
    //
    void airacceleration_fix();
    void apply_acceleration(const std::optional<float>& new_value = std::nullopt);

    // create-move
    //
    void bhop(c_usercmd* cmd);
    void autostrafer(c_usercmd* cmd);
    void portal_any_surface();
    void portal_boost();

    inline void create_move(c_usercmd* cmd) {
        autostrafer(cmd);
        bhop(cmd);
        portal_any_surface();
        portal_boost();
    }
} // namespace hack::features::misc