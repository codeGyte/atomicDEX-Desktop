/******************************************************************************
 * Copyright © 2013-2019 The Komodo Platform Developers.                      *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * Komodo Platform software, including this file may be copied, modified,     *
 * propagated or distributed except according to the terms contained in the   *
 * LICENSE file                                                               *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

#include <filesystem>
#include <imgui.h>
#include <antara/gaming/graphics/component.canvas.hpp>
#include <antara/gaming/event/quit.game.hpp>
#include <antara/gaming/event/key.pressed.hpp>
#include <IconsFontAwesome5.h>
#include "atomic.dex.gui.hpp"
#include "atomic.dex.gui.widgets.hpp"
#include "atomic.dex.mm2.hpp"

namespace fs = std::filesystem;
// Helpers
namespace {
    ImVec4 bright_color{0, 149.f / 255.f, 143.f / 255.f, 1};
    ImVec4 dark_color{25.f / 255.f, 40.f / 255.f, 56.f / 255.f, 1};

    std::string usd_str(const std::string &amt) {
        return amt + " USD";
    }
}

namespace {
    static std::string curr_asset_code = "";
    static int selected = 0;
}

// GUI Draw
namespace {
    void gui_menubar() noexcept {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) { /* Do stuff */ }
            if (ImGui::MenuItem("Settings", "Ctrl+O")) { /* Do stuff */ }
            ImGui::EndMenuBar();
        }
    }

    void gui_portfolio_coins_list(atomic_dex::mm2 &mm2) noexcept {
        ImGui::BeginChild("left pane", ImVec2(180, 0), true);
        int i = 0;
        auto assets_contents = mm2.get_enabled_coins();
        for (auto it = assets_contents.begin(); it != assets_contents.end(); ++it, ++i) {
            auto &asset = *it;
            if (curr_asset_code == "") curr_asset_code = asset.ticker;
//            ImGui::Image(icons.at(asset.ticker));
            //ImGui::SameLine();
            if (ImGui::Selectable(asset.name.c_str(), selected == i)) {
                selected = i;
                curr_asset_code = asset.ticker;
            }
        }
        ImGui::EndChild();
    }

    void gui_portfolio_coin_details(atomic_dex::mm2 &mm2) noexcept {
        // Right
        const auto curr_asset = mm2.get_coin_info(curr_asset_code);
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                          true); // Leave room for 1 line below us
        {
            ImGui::TextWrapped("%s", curr_asset.name.c_str());
            ImGui::Separator();
            std::error_code ec;

            ImGui::Text(std::string(std::string(ICON_FA_BALANCE_SCALE) + " Balance: %s %s (%s)").c_str(),
                        mm2.my_balance(curr_asset.ticker, ec).c_str(),
                        curr_asset.ticker.c_str(), "0");
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
                if (ImGui::BeginTabItem("Transactions")) {
                    std::error_code ec;
                    auto tx_history = mm2.get_tx_history(curr_asset.ticker, ec);
                    if (tx_history.size() > 0) {
                        for (std::size_t i = 0; i < tx_history.size(); ++i) {
                            auto &tx = tx_history[i];
                            ImGui::Text("%s", tx.am_i_sender ? "Sent" : "Received");
                            ImGui::SameLine(300);
                            ImGui::TextColored(
                                    ImVec4(tx.am_i_sender ? ImVec4(1, 52.f / 255.f, 0, 1.f) : ImVec4(80.f / 255.f, 1,
                                                                                                     118.f / 255.f,
                                                                                                     1.f)),
                                    "%s%s %s", tx.am_i_sender ? "-" : "+", tx.my_balance_change.c_str(),
                                    curr_asset.ticker.c_str());
                            ImGui::TextColored(ImVec4(128.f / 255.f, 128.f / 255.f, 128.f / 255.f, 1.f), "%s",
                                               tx.am_i_sender ? tx.to[0].c_str() : tx.from[0].c_str());
                            ImGui::SameLine(300);
                            ImGui::TextColored(ImVec4(128.f / 255.f, 128.f / 255.f, 128.f / 255.f, 1.f), "%s",
                                               usd_str("1234").c_str());
                            if (i != tx_history.size() - 1) ImGui::Separator();
                        }
                    } else ImGui::Text("No transactions");

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Receive")) {
                    ImGui::Text("Work in progress, will receive coins here");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Send")) {
                    ImGui::Text("Work in progress, will send coins here");
                    if (ImGui::Button("Send")) {

                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::EndChild();
    }

    void gui_enable_coins() {
        if (ImGui::Button("Enable a coin"))
            ImGui::OpenPopup("Enable coins");

        if (ImGui::BeginPopupModal("Enable coins", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
            ImGui::Separator();

            static int dummy_i = 0;
            ImGui::Combo("Combo", &dummy_i, "Delete\0Delete harder\0");

            static bool dont_ask_me_next_time = false;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
            ImGui::PopStyleVar();

            if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }
    }

    void gui_portfolio(atomic_dex::mm2 &mm2) noexcept {
        ImGui::Text("Total Balance: %s", usd_str("1337").c_str());

        gui_enable_coins();

        // Left
        gui_portfolio_coins_list(mm2);

        // Right
        ImGui::SameLine();
        gui_portfolio_coin_details(mm2);
    }
}

namespace atomic_dex {
#if defined(ENABLE_CODE_RELOAD_UNIX)
    //! Platform dependent code
    class AtomicDexHotCodeListener : public jet::ILiveListener
    {
    public:
        void onLog(jet::LogSeverity severity, const std::string &message) override
        {
            std::string severityString;
            auto verbosity = loguru::Verbosity_INFO;
            switch (severity) {
                case jet::LogSeverity::kDebug:
                    severityString.append("[D]");
                    verbosity = loguru::Verbosity_INFO;
                    break;
                case jet::LogSeverity::kInfo:
                    severityString.append("[I]");
                    verbosity = loguru::Verbosity_INFO;
                    break;
                case jet::LogSeverity::kWarning:
                    severityString.append("[W]");
                    verbosity = loguru::Verbosity_WARNING;
                    break;
                case jet::LogSeverity::kError:
                    severityString.append("[E]");
                    verbosity = loguru::Verbosity_ERROR;
                    break;
            }
            DVLOG_F(verbosity, "{}", message);
        }
    };
#endif

    void gui::reload_code() {
        DVLOG_F(loguru::Verbosity_INFO, "reloading code");
#if defined(ENABLE_CODE_RELOAD_UNIX)
        live_->tryReload();
#endif
    }

    void gui::init_live_coding() {
#if defined(ENABLE_CODE_RELOAD_UNIX)
        live_ = jet::make_unique<jet::Live>(jet::make_unique<AtomicDexHotCodeListener>());
        while (!live_->isInitialized()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            live_->update();
        }
        live_->update();
#endif
    }

    void gui::update_live_coding() {
#if defined(ENABLE_CODE_RELOAD_UNIX)
        live_->update();
#endif
    }
}

namespace atomic_dex {
    void gui::on_key_pressed(const ag::event::key_pressed &evt) noexcept {
        if (evt.key == ag::input::r && evt.control) {
            reload_code();
        }
    }

    gui::gui(entt::registry &registry, atomic_dex::mm2 &mm2_system) noexcept : system(registry),
                                                                               mm2_system_(mm2_system) {
        init_live_coding();
        atomic_dex::style::apply();
        this->dispatcher_.sink<ag::event::key_pressed>().connect<&gui::on_key_pressed>(*this);
    }

    void gui::update() noexcept {
        update_live_coding();

        //! Menu bar
        auto &canvas = entity_registry_.ctx<ag::graphics::canvas_2d>();
        auto[x, y] = canvas.window.size;
        auto[pos_x, pos_y] = canvas.window.position;

        ImGui::SetNextWindowSize(ImVec2(x, y), ImGuiCond_Once);
        //ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y));
        bool active = true;
        ImGui::Begin("atomicDEX", &active,ImGuiWindowFlags_NoCollapse);
        if (not active && mm2_system_.is_mm2_running()) { this->dispatcher_.trigger<ag::event::quit_game>(0); }

        if (!mm2_system_.is_mm2_running()) {
            ImGui::Text("Loading, please wait...");
            const float radius = 30.0f;
            ImVec2 position((ImGui::GetWindowSize().x) * 0.5f - radius * 2,
                            (ImGui::GetWindowSize().y) * 0.5f - radius * 2);
            ImGui::SetCursorPos(position);
            atomic_dex::widgets::LoadingIndicatorCircle("foo", radius, ImVec4(bright_color), ImVec4(dark_color), 9,
                                                        1.5f);
        } else {
            gui_menubar();

            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
                if (ImGui::BeginTabItem("Portfolio")) {
                    gui_portfolio(mm2_system_);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Trade")) {
                    ImGui::Text("Work in progress");
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }

        ImGui::End();
    }
}
