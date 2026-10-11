#include "misc.hpp"

#include "hack/cfg/opts.hpp"

namespace hack::features::misc {
    namespace {
        inline const char* kPromoConvars[] = {
            "gameextras/skins",
            "gameextras/helmets",
            "gameextras/antenna",
        };
    } // namespace

    // Promo skins/helmets/antenna: the game itself reads these server convars
    // (`cmp [convar + 0x30], 0` in server.dll) and grants the items on spawn.
    // Same proven int-write mechanism as portal_any_surface. A respawn or
    // changelevel may be needed for the tabs to show them.
    void apply_robot_unlock() {
        if (!opts::robot_unlock_all)
            return;

        for (auto* name : kPromoConvars) {
            if (auto* cv = portal::cvar->find_convar(name))
                cv->m_int_value = 1;
        }
    }
} // namespace hack::features::misc
