#include "misc.hpp"

#include "hack/cfg/opts.hpp"
#include "hack/menu/menu.hpp"
#include <cmath>
#include <cstdio>

namespace hack::features::misc {
    void jump_tracker(c_usercmd* cmd) {
        if (!opts::jump_stats) {
            jump::has_takeoff = false;
            jump::has_result = false;
            return;
        }

        if (!portal::players::local.get()) {
            jump::has_takeoff = false;
            return;
        }

        auto* local = portal::players::local.operator->();
        if (!local) {
            jump::has_takeoff = false;
            return;
        }

        static bool was_onground = true;
        const bool onground = (local->m_fFlags() & e_ent_flags::fl_onground) != 0;
        const vec3_t pos = local->m_vecOrigin();

        if (was_onground && !onground) {
            // left the ground: count it as a jump only if jump was pressed
            if (cmd->m_buttons & e_cmd_buttons::in_jump) {
                jump::takeoff_pos = pos;
                jump::takeoff_speed = local->m_vecVelocity().length_2d();
                jump::has_takeoff = true;
                jump::has_result = false;
            } else {
                jump::has_takeoff = false; // just walked/fell off
            }
        }

        if (!was_onground && onground && jump::has_takeoff) {
            const float dx = pos.x - jump::takeoff_pos.x;
            const float dy = pos.y - jump::takeoff_pos.y;
            jump::last_distance = std::sqrt(dx * dx + dy * dy);
            jump::last_speed = jump::takeoff_speed;
            jump::has_result = true;
            jump::has_takeoff = false;
        }

        was_onground = onground;
    }

    void speedometer_overlay() {
        if (!opts::speedometer && !opts::jump_stats && !opts::button_glitch)
            return;

        if (!portal::players::local.get())
            return;

        auto* local = portal::players::local.operator->();
        if (!local)
            return;

        constexpr ImGuiWindowFlags kFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                                            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs;

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("##speedo", nullptr, kFlags))
            return ImGui::End();

        if (opts::speedometer)
            ImGui::Text("%.0f u/s", static_cast<double>(local->m_vecVelocity().length_2d()));

        if (opts::jump_stats && jump::has_result)
            ImGui::Text("last jump: %.0f u @ %.0f u/s", static_cast<double>(jump::last_distance), static_cast<double>(jump::last_speed));

        if (opts::button_glitch) {
            if (button::touching)
                ImGui::Text("button: %d tick", button::touch_ticks);
            else if (button::has_last_touch)
                ImGui::Text("last touch: %d ticks", button::last_touch_ticks);
        }

        ImGui::End();
    }
} // namespace hack::features::misc
