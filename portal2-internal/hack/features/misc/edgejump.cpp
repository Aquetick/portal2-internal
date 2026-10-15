#include "misc.hpp"

#include "hack/cfg/opts.hpp"

namespace hack::features::misc {
    // Edge jump assist: jumps on the exact tick you leave the ground.
    // Pure input timing (sets the jump bit in the usercmd), no cheats.
    // Runs after bhop so the added jump isn't stripped the same tick.
    void edge_jump(c_usercmd* cmd) {
        static bool was_onground = true;

        if (!portal::players::local.get()) {
            was_onground = true;
            return;
        }

        auto* local = portal::players::local.operator->();
        if (!local) {
            was_onground = true;
            return;
        }

        bool hold_ok = true;
        if (opts::edgejump_hold) {
            const int key = opts::edgejump_key;
            hold_ok = (key > 0 && key < 256) && util::input::get(key).m_state != util::input::e_button_state::idle;
        }

        const bool onground = (local->m_fFlags() & e_ent_flags::fl_onground) != 0;

        if (opts::edgejump && hold_ok && was_onground && !onground)
            cmd->m_buttons |= e_cmd_buttons::in_jump;

        was_onground = onground;
    }
} // namespace hack::features::misc
