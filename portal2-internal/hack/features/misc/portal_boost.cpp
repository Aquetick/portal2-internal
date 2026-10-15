#include "misc.hpp"

#include "hack/cfg/opts.hpp"
#include <cmath>

namespace hack::features::misc {
    // Reportal-style portal boost.
    // Idea: teleport through a portal appears as a large origin jump
    // between two consecutive CreateMove ticks. When such a jump is
    // detected while the boost bind is held, exit velocity is scaled
    // by opts::portal_boost_multiplier (2x..8x).
    // Works for walk-in / fly-in alike, no portal entity lookup needed.
    void portal_boost() {
        if (!opts::portal_boost)
            return;

        if (!portal::players::local.get())
            return;

        auto* local = portal::players::local.operator->();
        if (!local)
            return;

        // hold-to-boost: only active while the bind is down.
        // (mirrors portal_any_surface hold logic)
        bool active = true;
        if (opts::portal_boost_hold) {
            const int key = opts::portal_boost_key;
            if (key <= 0 || key >= 256)
                return;
            active = util::input::get(key).m_state != util::input::e_button_state::idle;
        }
        if (!active)
            return;

        float mult = opts::portal_boost_multiplier;
        if (mult < 2.f)
            mult = 2.f;
        else if (mult > 8.f)
            mult = 8.f;

        static bool has_last = false;
        static vec3_t last_origin{};
        static int cooldown = 0;

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

        if (cooldown > 0) {
            --cooldown;
            return;
        }

        if (!active)
            return;

        // Normal per-tick movement is a few units (even fast fall at
        // 3000 u/s is ~50u per tick at 60hz). A portal teleport is
        // hundreds/thousands of units, so 80u is a safe threshold.
        constexpr float kTeleportDist = 80.f;
        if (dist < kTeleportDist)
            return;

        vec3_t& vel = local->m_vecVelocity();
        if (vel.length() < 0.5f)
            return;

        vel.x *= mult;
        vel.y *= mult;
        vel.z *= mult;

        // Ignore the next ticks so the boosted high-speed movement
        // itself is not mistaken for another teleport.
        cooldown = 30;
    }
} // namespace hack::features::misc
