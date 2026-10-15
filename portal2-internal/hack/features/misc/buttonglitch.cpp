#include "misc.hpp"

#include "hack/cfg/opts.hpp"
#include <cctype>

namespace hack::features::misc {
    namespace {
        // case-insensitive substring match on the client class name,
        // so both prop_floor_button and prop_under_button (and any
        // future button class) are picked up without hardcoding names.
        bool class_has(c_clientclass* cc, const char* sub) {
            if (!cc || !cc->m_network_name)
                return false;
            for (const char* n = cc->m_network_name; *n; ++n) {
                const char* a = n;
                const char* b = sub;
                while (*a && *b && std::tolower(static_cast<unsigned char>(*a)) == *b) {
                    ++a;
                    ++b;
                }
                if (!*b)
                    return true;
            }
            return false;
        }

        // geometric touch check: feet within the button plate footprint.
        // No netvars needed - origin offsets are universal.
        bool feet_on_button(c_base_player* local) {
            const vec3_t feet = local->m_vecOrigin();

            const int hi = portal::entitylist->get_highest_entity_index();
            if (hi <= 0 || hi > 4096)
                return false;

            for (int i = 0; i <= hi; ++i) {
                c_base_entity* e = portal::entitylist->get_client_entity(i);
                if (!e || e == reinterpret_cast<c_base_entity*>(local))
                    continue;
                if (!class_has(e->get_client_class(), "button"))
                    continue;

                const vec3_t o = e->m_vecOrigin();
                const float dx = feet.x - o.x;
                const float dy = feet.y - o.y;
                const float dz = feet.z - o.z;
                if (dx * dx + dy * dy > 40.f * 40.f)
                    continue;
                if (dz < -8.f || dz > 34.f)
                    continue;
                return true;
            }
            return false;
        }
    } // namespace

    // 1-tick button glitch helper.
    // While the bind is held, counts ticks spent touching a floor/under
    // button (HUD shows the live counter) and auto step-offs (jump) after
    // N ticks, so the server only ever sees a short touch - the exact
    // condition the sticky-button glitch needs. Pure client input, the
    // same primitive as bhop/edge jump, no sv_cheats involved.
    void button_glitch(c_usercmd* cmd) {
        if (!opts::button_glitch) {
            button::touching = false;
            button::touch_ticks = 0;
            return;
        }

        if (!portal::players::local.get()) {
            button::touching = false;
            button::touch_ticks = 0;
            return;
        }

        auto* local = portal::players::local.operator->();
        if (!local) {
            button::touching = false;
            button::touch_ticks = 0;
            return;
        }

        bool armed = true;
        if (opts::button_glitch_hold) {
            const int key = opts::button_glitch_key;
            armed = (key > 0 && key < 256) && util::input::get(key).m_state != util::input::e_button_state::idle;
        }

        const bool onground = (local->m_fFlags() & e_ent_flags::fl_onground) != 0;
        const bool touching = onground && feet_on_button(local);

        if (touching) {
            button::touching = true;
            ++button::touch_ticks;
        } else {
            if (button::touching && button::touch_ticks > 0) {
                button::last_touch_ticks = button::touch_ticks;
                button::has_last_touch = true;
            }
            button::touching = false;
            button::touch_ticks = 0;
        }

        int n = opts::button_touch_ticks;
        if (n < 1)
            n = 1;
        else if (n > 5)
            n = 5;

        if (armed && touching && button::touch_ticks >= n)
            cmd->m_buttons |= e_cmd_buttons::in_jump;
    }
} // namespace hack::features::misc
