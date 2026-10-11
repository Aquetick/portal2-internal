#include "misc.hpp"

#include "hack/cfg/opts.hpp"

namespace hack::features::misc {
    // sv_portal_placement_never_fail (+never_bump) live at int +0x30 of the
    // ConVar (proven in server.dll: `cmp dword ptr [convar + 0x30], 0`).
    // Written only on active/inactive transitions, never every tick.
    void portal_any_surface() {
        static i_convar* never_fail = nullptr;
        static i_convar* never_bump = nullptr;
        if (!never_fail || !never_bump) {
            never_fail = portal::cvar->find_convar("sv_portal_placement_never_fail");
            never_bump = portal::cvar->find_convar("sv_portal_placement_never_bump");
            if (!never_fail || !never_bump)
                return;
        }

        bool active = opts::portal_any_surface;
        if (!active && opts::portal_any_surface_hold) {
            const int key = opts::portal_any_surface_key;
            if (key > 0 && key < 256)
                active = util::input::get(key).m_state != util::input::e_button_state::idle;
        }

        static bool applied = false;
        static bool have_applied = false;
        if (!have_applied || applied != active) {
            never_fail->m_int_value = active ? 1 : 0;
            never_bump->m_int_value = active ? 1 : 0;
            applied = active;
            have_applied = true;
        }
    }
} // namespace hack::features::misc
