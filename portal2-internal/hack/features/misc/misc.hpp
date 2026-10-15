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
    void edge_jump(c_usercmd* cmd);
    void jump_tracker(c_usercmd* cmd);
    void button_glitch(c_usercmd* cmd);
    void quick_points();
    void render_tweaks();

    // present (overlay)
    //
    void speedometer_overlay();

    // last-jump state (tracker writes, overlay reads)
    //
    namespace jump {
        inline bool has_takeoff = false;
        inline vec3_t takeoff_pos{};
        inline float takeoff_speed = 0.f;
        inline float last_distance = 0.f;
        inline float last_speed = 0.f;
        inline bool has_result = false;
    } // namespace jump

    // button glitch helper state (detector writes, overlay reads)
    //
    namespace button {
        inline bool touching = false;
        inline int touch_ticks = 0;
        inline int last_touch_ticks = 0;
        inline bool has_last_touch = false;
    } // namespace button

    inline void create_move(c_usercmd* cmd) {
        autostrafer(cmd);
        bhop(cmd);
        portal_any_surface();
        edge_jump(cmd);
        jump_tracker(cmd);
        button_glitch(cmd);
        quick_points();
        render_tweaks();
    }

    inline void present() {
        speedometer_overlay();
    }
} // namespace hack::features::misc