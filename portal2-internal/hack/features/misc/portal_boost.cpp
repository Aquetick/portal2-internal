#include "misc.hpp"

#include "hack/cfg/opts.hpp"
#include <cmath>
#include <cstdio>

namespace hack::features::misc {
    // Reportal-style portal boost, server-authoritative edition.
    //
    // NOTE: scaling m_vecVelocity on the client alone does nothing
    // visible - even in single-player the listen server re-simulates
    // movement and corrects prediction back. So the real boost is a
    // short phys_timescale surge on the server: physics (including
    // the player) briefly runs at multiplier speed right after a
    // portal teleport. (Client velocity is still scaled too - it only
    // ever lasts a tick, but it is harmless.)
    //
    // Teleport detection: passing through a portal appears as a large
    // origin jump between two consecutive CreateMove ticks. Works for
    // walk-in / fly-in alike, no portal entity lookup needed.
    void portal_boost() {
        static i_convar* timescale = nullptr;
        static bool timescale_lookup_done = false;
        static bool has_last = false;
        static vec3_t last_origin{};
        static int cooldown = 0;    // post-trigger guard (ticks)
        static int pulse_ticks = 0; // active surge countdown (ticks)
        static bool applied = false;

        const auto restore = []() {
            if (applied && timescale)
                timescale->m_float_value = 1.f;
            applied = false;
            pulse_ticks = 0;
        };

        if (!opts::portal_boost) {
            restore();
            has_last = false;
            cooldown = 0;
            return;
        }

        if (!timescale_lookup_done) {
            timescale = portal::cvar->find_convar("phys_timescale");
            timescale_lookup_done = true;
        }

        if (!portal::players::local.get()) {
            restore();
            has_last = false;
            return;
        }

        auto* local = portal::players::local.operator->();
        if (!local) {
            restore();
            has_last = false;
            return;
        }

        // hold-to-boost: only active while the bind is down.
        // (mirrors portal_any_surface hold logic)
        bool active = true;
        if (opts::portal_boost_hold) {
            const int key = opts::portal_boost_key;
            if (key <= 0 || key >= 256) {
                restore();
                return;
            }
            active = util::input::get(key).m_state != util::input::e_button_state::idle;
        }

        float mult = opts::portal_boost_multiplier;
        if (mult < 2.f)
            mult = 2.f;
        else if (mult > 8.f)
            mult = 8.f;

        float dur = opts::portal_boost_duration;
        if (dur < 0.1f)
            dur = 0.1f;
        else if (dur > 1.f)
            dur = 1.f;

        vec3_t cur = local->m_vecOrigin();
        if (!has_last) {
            last_origin = cur;
            has_last = true;
            return;
        }

        const float dx = cur.x - last_origin.x;
        const float dy = cur.y - last_origin.y;
        const float dz = cur.z - last_origin.z;
        const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        last_origin = cur;

        if (cooldown > 0)
            --cooldown;

        // surge in progress: count down, restore timescale at the end
        if (pulse_ticks > 0) {
            if (--pulse_ticks == 0)
                restore();
            return;
        }

        if (cooldown > 0 || !active)
            return;

        // Normal per-tick movement is a few units (even a 3000 u/s
        // fall is ~45u per tick). A portal teleport is hundreds /
        // thousands of units, so 80u is a safe threshold.
        constexpr float kTeleportDist = 80.f;
        if (dist < kTeleportDist)
            return;

        if (!timescale)
            return; // fail-safe: convar missing, don't touch anything

        vec3_t& vel = local->m_vecVelocity();
        if (vel.length() < 0.5f)
            return;

        vel.x *= mult;
        vel.y *= mult;
        vel.z *= mult;

        timescale->m_float_value = mult;
        applied = true;
        pulse_ticks = static_cast<int>(dur * 66.f);
        if (pulse_ticks < 1)
            pulse_ticks = 1;
        cooldown = pulse_ticks + 30;

        char msg[64]{};
        snprintf(msg, sizeof(msg), "echo [portal-boost] x%.1f surge", static_cast<double>(mult));
        portal::engine_client->server_cmd(msg);
    }
} // namespace hack::features::misc
