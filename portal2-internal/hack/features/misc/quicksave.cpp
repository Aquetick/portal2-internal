#include "misc.hpp"

#include "hack/cfg/opts.hpp"

namespace hack::features::misc {
    // Practice checkpoints without sv_cheats: `save`/`load` are regular
    // single-player commands (F6/F6 in vanilla), executed on the server
    // via the guarded ServerCmd path.
    void quick_points() {
        const int save = opts::quicksave_key;
        if (save > 0 && save < 256 && util::input::get(save).pressed())
            portal::engine_client->server_cmd("save quick");

        const int load = opts::quickload_key;
        if (load > 0 && load < 256 && util::input::get(load).pressed())
            portal::engine_client->server_cmd("load quick");
    }
} // namespace hack::features::misc
