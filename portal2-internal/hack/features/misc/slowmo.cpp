#include "misc.hpp"

#include "hack/cfg/opts.hpp"

namespace hack::features::misc {
    // Manual slow motion for practicing timings (reportal setups, mid-air
    // portals). Writes phys_timescale directly - same proven pattern as
    // sv_paintairacceleration / portal_any_surface, no sv_cheats flip.
    // NOTE: as host it slows the whole server sim, not just you.
    void slowmo() {
        static i_convar* timescale = nullptr;
        static bool looked = false;
        static bool applied = false;
        static float set_v = 1.f;

        if (!looked) {
            timescale = portal::cvar->find_convar("phys_timescale");
            looked = true;
        }

        float v = opts::slowmo_value;
        if (v < 0.1f)
            v = 0.1f;
        else if (v > 2.f)
            v = 2.f;

        bool want = opts::slowmo;
        if (want && opts::slowmo_hold) {
            const int key = opts::slowmo_key;
            want = (key > 0 && key < 256) && util::input::get(key).m_state != util::input::e_button_state::idle;
        }

        if (want && timescale) {
            if (!applied || set_v != v) {
                timescale->m_float_value = v;
                applied = true;
                set_v = v;
            }
        } else if (applied) {
            if (timescale)
                timescale->m_float_value = 1.f;
            applied = false;
        }
    }
} // namespace hack::features::misc
