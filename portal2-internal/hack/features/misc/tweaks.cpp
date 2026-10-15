#include "misc.hpp"

#include "hack/cfg/opts.hpp"

namespace hack::features::misc {
    // Local render tweaks. Client-side material/fog state only, applied
    // on change like portal_any_surface - no sv_cheats involved.
    void render_tweaks() {
        static i_convar* fullbright = nullptr;
        static i_convar* fog = nullptr;
        static bool looked = false;
        static bool inited = false;
        static bool fb_set = false;
        static bool fog_set = false;

        if (!looked) {
            fullbright = portal::cvar->find_convar("mat_fullbright");
            fog = portal::cvar->find_convar("fog_enable");
            looked = true;
        }

        if (fullbright && (!inited || fb_set != opts::fullbright)) {
            fullbright->m_int_value = opts::fullbright ? 1 : 0;
            fb_set = opts::fullbright;
        }

        if (fog && (!inited || fog_set != opts::nofog)) {
            fog->m_int_value = opts::nofog ? 0 : 1;
            fog_set = opts::nofog;
        }

        inited = true;
    }
} // namespace hack::features::misc
