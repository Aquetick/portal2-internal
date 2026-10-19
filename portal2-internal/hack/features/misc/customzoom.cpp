#include "misc.hpp"

#include "hack/cfg/opts.hpp"
#include <cmath>

namespace hack::features::misc {
    // custom zoom: toggle on key press, not hold.
    // While zoomed, fov_value is overridden with custom_zoom_fov and the
    // client sensitivity (sensitivity convar) is scaled by
    // custom_zoom_sens. On release, both are restored.
    //
    // FOV is applied in override_view (client-side, visual only, no
    // server side), sensitivity is applied through the convar (proven
    // pattern used by airacceleration / portal_any_surface).
    namespace zoom {
        inline bool zoomed = false;
        inline bool prev_down = false;
        inline i_convar* sens = nullptr;
        inline bool sens_looked = false;
        inline float sens_base = 0.f;
        inline bool sens_saved = false;
    } // namespace zoom

    void custom_zoom() {
        if (!opts::custom_zoom) {
            zoom::zoomed = false;
            return;
        }

        const int key = opts::custom_zoom_key;
        if (key <= 0 || key >= 256)
            return;

        const bool down = util::input::get(key).m_state != util::input::e_button_state::idle;
        if (down && !zoom::prev_down)
            zoom::zoomed = !zoom::zoomed; // toggle on press edge
        zoom::prev_down = down;

        if (!zoom::sens_looked) {
            zoom::sens = portal::cvar->find_convar("sensitivity");
            zoom::sens_looked = true;
        }

        if (zoom::sens) {
            float v = opts::custom_zoom_sens;
            if (v < 0.1f)
                v = 0.1f;
            else if (v > 1.f)
                v = 1.f;

            if (zoom::zoomed) {
                if (!zoom::sens_saved) {
                    zoom::sens_base = zoom::sens->m_float_value;
                    zoom::sens_saved = true;
                }
                zoom::sens->m_float_value = zoom::sens_base * v;
            } else if (zoom::sens_saved) {
                zoom::sens->m_float_value = zoom::sens_base;
                zoom::sens_saved = false;
            }
        }
    }
} // namespace hack::features::misc
