#include "menu.hpp"
#include "hack/cfg/cfg.hpp"
#include "hack/hack.hpp"
#include "util/util.hpp"
#include <Windows.h>
#include <cstdio>
#include <string>

namespace hack::menu {
    namespace {
        // small key picker button, stores picked key into *key
        inline void key_picker(const char* id, int* key, bool& picking) {
            ImGui::PushID(id);

            auto key_name = [](int vk) -> std::string {
                if (vk >= 0x41 && vk <= 0x5A)
                    return std::string(1, static_cast<char>(vk));
                if (vk >= 0x30 && vk <= 0x39)
                    return std::string(1, static_cast<char>(vk));
                if (vk == VK_XBUTTON1)
                    return "Mouse4";
                if (vk == VK_XBUTTON2)
                    return "Mouse5";
                if (vk == VK_MBUTTON)
                    return "Mouse3";
                char buf[16]{};
                snprintf(buf, sizeof(buf), "0x%02X", vk);
                return buf;
            };

            if (ImGui::Button(picking ? "..." : key_name(*key).c_str()))
                picking = true;

            if (picking) {
                for (int vk = 0x08; vk <= 0xFE; ++vk) {
                    if (vk == VK_INSERT || vk == VK_F2)
                        continue;
                    if (util::input::get(vk).m_state == util::input::e_button_state::pressed) {
                        *key = vk;
                        util::input::get(vk).m_state = util::input::e_button_state::held;
                        picking = false;
                        break;
                    }
                }
            }

            ImGui::PopID();
        }
    } // namespace
} // namespace hack::menu

namespace hack::menu {
    constexpr float_t kDefaultAirAccelerationValue = 5.f;
    constexpr float_t kCSGOAirAccelerationValue = 12.f;
    constexpr float_t kBhopAirAccelerationValue = 10000.f;
    constexpr float_t kDefaultFOVValue = 90.f;

    namespace {
        inline bool color_edit(const char* label, color_t* col, ImGuiColorEditFlags flags, const char* id = nullptr) {
            ImGui::PushID(!id ? label : id);
            auto clr = ImVec4(col->r / 255.f, col->g / 255.f, col->b / 255.f, col->a / 255.f);

            ImGuiColorEditFlags color_button_flags = ImGuiColorEditFlags_NoTooltip;
            if (!(flags & ImGuiColorEditFlags_NoAlpha))
                color_button_flags |= ImGuiColorEditFlags_AlphaPreview;

            bool openPopup = ImGui::ColorButton("##btn", clr, color_button_flags);
            ImGui::SameLine();
            ImGui::Text(label);

            if (openPopup)
                ImGui::OpenPopup("##popup");

            if (ImGui::BeginPopup("##popup")) {
                ImGuiColorEditFlags color_picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs;
                if (!(flags & ImGuiColorEditFlags_NoAlpha))
                    color_picker_flags |= ImGuiColorEditFlags_AlphaBar;

                if (ImGui::ColorPicker4("##picker", &clr.x, color_picker_flags))
                    col->apply(static_cast<color_t::channel_flt_t>(clr.x), static_cast<color_t::channel_flt_t>(clr.y),
                               static_cast<color_t::channel_flt_t>(clr.z), static_cast<color_t::channel_flt_t>(clr.w));

                ImGui::Separator();
                ImGui::BeginColumns("##colsinpopup", 4, ImGuiOldColumnFlags_NoBorder);
                ImGui::Text("R: %d", col->r);
                ImGui::NextColumn();
                ImGui::Text("G: %d", col->g);
                ImGui::NextColumn();
                ImGui::Text("B: %d", col->b);
                ImGui::NextColumn();
                ImGui::Text("A: %d", col->a);
                ImGui::EndColumns();
                ImGui::Separator();

                ImGui::Checkbox("Rainbow", &col->rainbow.enabled);

                ImGui::EndPopup();
            }

            ImGui::PopID();
            return false;
        }
    } // namespace

    void render() {
        if (!opened)
            return;

        ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize * 0.5f, ImGuiCond_Once, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(kMenuWidth, kMenuHeight), ImGuiCond_Once);

        if (!ImGui::Begin("portal2 cheeto | INSERT / F2", &opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
            return ImGui::End();

        static bool first_render{true};
        static int tab = 0;
        static bool picking_any = false;

        // tab bar (note: never mutate `tab` inside the loop -
        // Push/PopStyleColor must stay balanced within one iteration)
        const char* tabs[] = {"Movement", "Portals", "Visuals", "Config"};
        int pending_tab = -1;
        for (int i = 0; i < 4; ++i) {
            const bool is_active = (i == tab);
            if (i > 0)
                ImGui::SameLine();
            if (is_active)
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
            if (ImGui::Button(tabs[i], ImVec2((kMenuWidth - 32.f) / 4.f, 24.f)))
                pending_tab = i;
            if (is_active)
                ImGui::PopStyleColor();
        }
        if (pending_tab >= 0)
            tab = pending_tab;

        ImGui::Separator();
        ImGui::BeginChild("##page", ImVec2(0, 0), false);

        if (tab == 0) {
            ImGui::Text("Movement");
            ImGui::Separator();

            ImGui::Checkbox("bhop", &opts::bhop);
            ImGui::Checkbox("autostrafer", &opts::autostrafer);
            if (ImGui::Checkbox("airacceleration_fix", &opts::airacceleration_fix) || (first_render && opts::airacceleration_fix))
                hack::features::misc::airacceleration_fix();

            ImGui::Separator();

            ImGui::SetNextItemWidth(120.f);
            if (ImGui::InputFloat("airacceleration", &opts::airacceleration_value, 1.f, 5.f, "%1.f") ||
                (first_render && std::fabsf(opts::airacceleration_value - kDefaultAirAccelerationValue) > 0.01f))
                features::misc::apply_acceleration();

            auto custom_acceleration_btn = [](const char* name, const float_t value) [[msvc::forceinline]] -> void {
                ImGui::SameLine();
                if (!ImGui::Button(name))
                    return;
                features::misc::apply_acceleration(value);
            };

            custom_acceleration_btn("DEFAULT", kDefaultAirAccelerationValue);
            custom_acceleration_btn("CSGO", kCSGOAirAccelerationValue);
            custom_acceleration_btn("BHOP", kBhopAirAccelerationValue);

            ImGui::Separator();

            ImGui::SliderFloat("FOV", &opts::fov_value, 10.f, 160.f);
            ImGui::SameLine();
            if (ImGui::Button("Reset"))
                opts::fov_value = kDefaultFOVValue;
        } else if (tab == 1) {
            ImGui::Text("Portals");
            ImGui::Separator();

            ImGui::Checkbox("any surface (always)", &opts::portal_any_surface);
            ImGui::Checkbox("any surface (hold)", &opts::portal_any_surface_hold);
            ImGui::Text("key:");
            ImGui::SameLine();
            key_picker("any", &opts::portal_any_surface_key, picking_any);
            ImGui::SameLine();
            if (ImGui::Button("Reset##anykey"))
                opts::portal_any_surface_key = 0x58; // X

            ImGui::Separator();

            // @fixme: @es3n1n: remove prefix_col_1/prefix_col_2
            const auto portal_colors_settings = [](const std::string_view prefix, std::size_t index, const std::string_view prefix_col_1,
                                                   const std::string_view prefix_col_2) [[msvc::forceinline]] -> void {
                ImGui::PushID(prefix.data());

                ImGui::Text(prefix.data());
                ImGui::SameLine();

                color_edit("Portal 1", &opts::portal_colors[index].m_portal_1, ImGuiColorEditFlags_NoAlpha, prefix_col_1.data());
                ImGui::SameLine();
                color_edit("Portal 2", &opts::portal_colors[index].m_portal_2, ImGuiColorEditFlags_NoAlpha, prefix_col_2.data());

                ImGui::PopID();
            };

            portal_colors_settings("Player1:", 0, "p1c1", "p1c2");
            portal_colors_settings("Player2:", 1, "p2c1", "p2c2");
        } else if (tab == 2) {
            ImGui::Text("Visuals");
            ImGui::Separator();

            auto chams_settings = [=](const std::string_view name, opts::chams_opts_t* ptr) [[msvc::forceinline]] -> void {
                ImGui::PushID(name.data());
                ImGui::Checkbox(name.data(), &ptr->m_enabled);
                ImGui::SameLine();
                color_edit("Color", &ptr->m_color, 0);
                ImGui::SameLine();

                ImGui::SetNextItemWidth(120.f);
                ImGui::Combo("Type", &ptr->m_material, "Normal\0Flat\0");

                ImGui::PopID();
            };

            chams_settings("Portalgun Chams", &opts::portal_gun_chams);
            chams_settings("Chell Chams", &opts::chell_chams);
            chams_settings("Wheatley Chams", &opts::wheatley_chams);

            ImGui::Separator();

            ImGui::Checkbox("Mat Ambient Light modification", &opts::mat_ambient_light_enabled);
            ImGui::SameLine();
            color_edit("Color", &opts::mat_ambient_light_value, 0, "matambient");

            ImGui::Separator();

            ImGui::Checkbox("Trails", &opts::trails);
            ImGui::SameLine();
            color_edit("Color", &opts::trails_color, 0, "trails");
            ImGui::SameLine();

            ImGui::SetNextItemWidth(120.f);
            ImGui::SliderFloat("Life time", &opts::trails_life_time, 1.f, 15.f);
        } else {
            ImGui::Text("Config");
            ImGui::Separator();

            if (ImGui::Button("save"))
                cfg::save("config");
            ImGui::SameLine();
            if (ImGui::Button("load"))
                cfg::read("config");

            ImGui::Separator();

            if (ImGui::Button("randomize rainbow"))
                cfg::randomize_rainbow();
            ImGui::SameLine();
            if (ImGui::Button("sync rainbow"))
                cfg::sync_rainbow();
        }

        ImGui::EndChild();

        ImGui::End();
        first_render = false;
    }

    void toggle() {
        bool button_pressed = false;
        for (auto vk : {VK_INSERT, VK_F2}) {
            button_pressed |= util::input::get(vk).pressed();

            if (button_pressed) {
                break;
            }
        }

        if (!button_pressed) {
            return;
        }

        opened ^= true;
        return opened ? util::game::lock_cursor() : util::game::unlock_cursor();
    }

    void setup() {
        auto& io = ImGui::GetIO();
        io.LogFilename = NULL;
        io.IniFilename = NULL;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // fix annoying cursor flickering

        ImGui::StyleColorsDark();

        // match the index.html preview: dark blue-grey theme
        auto& style = ImGui::GetStyle();
        style.WindowRounding = 8.f;
        style.FrameRounding = 4.f;
        style.WindowPadding = ImVec2(14.f, 14.f);

        auto& c = style.Colors;
        c[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.11f, 1.f);
        c[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.11f, 1.f);
        c[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.13f, 0.16f, 1.f);
        c[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.13f, 0.16f, 1.f);
        c[ImGuiCol_Button] = ImVec4(0.15f, 0.16f, 0.20f, 1.f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.17f, 0.20f, 0.28f, 1.f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.25f, 0.35f, 1.f);
        c[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.07f, 0.09f, 1.f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.10f, 0.11f, 0.15f, 1.f);
        c[ImGuiCol_CheckMark] = ImVec4(0.49f, 0.91f, 0.47f, 1.f);
        c[ImGuiCol_SliderGrab] = ImVec4(0.36f, 0.55f, 1.f, 1.f);
        c[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.62f, 1.f, 1.f);
    }
} // namespace hack::menu
