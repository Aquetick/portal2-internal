#include "misc.hpp"

#include "hack/cfg/opts.hpp"
#include <string>

namespace hack::features::misc {
    namespace {
        // exact item names from portal2_dlc1/scripts/items/items_game.txt
        inline const char* kGestures[] = {
            "Gesture Eggbot SitSpin",  "Gesture Eggbot Facepalm", "Gesture Eggbot Shrug",    "Gesture Eggbot Thumbsup",
            "Gesture Eggbot Fistpump", "Gesture Eggbot Pretzelwave", "Gesture Eggbot Spinjump", "Gesture Eggbot Selfspin",
            "Gesture Ballbot Facepalm", "Gesture Ballbot Shrug",   "Gesture Ballbot Thumbsup", "Gesture Ballbot Fistpump",
            "Gesture Ballbot Headspin", "Gesture Ballbot Dance",   "Gesture Ballbot Basketball1",
            "Gesture Ballbot Basketball2",
        };

        inline const char* kPromoConvars[] = {
            "gameextras/skins",
            "gameextras/helmets",
            "gameextras/antenna",
        };
    } // namespace

    // Promo skins/helmets/antenna: the game itself reads these server convars
    // (`cmp [convar + 0x30], 0` in server.dll) and grants the items on spawn.
    // Gestures: mp_earn_taunt (flags=0, needs no sv_cheats) executed on the
    // server. A respawn or changelevel may be needed for tabs to show promos.
    void apply_robot_unlock() {
        if (!opts::robot_unlock_all)
            return;

        for (auto* name : kPromoConvars) {
            if (auto* cv = portal::cvar->find_convar(name))
                cv->m_int_value = 1;
        }

        for (auto* gesture : kGestures)
            portal::engine_client->server_cmd(("mp_earn_taunt \"" + std::string(gesture) + "\"").c_str());
    }
} // namespace hack::features::misc
