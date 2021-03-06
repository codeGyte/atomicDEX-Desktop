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

//! QT
#include <QDebug>
#include <QLocale>

//! PCH
#include "atomic.dex.pch.hpp"

//! Deps
#include <boost/algorithm/string/case_conv.hpp>

//! Project Headers
#include "atomic.dex.events.hpp"
#include "atomic.dex.mm2.hpp"
#include "atomic.dex.qt.bindings.hpp"
#include "atomic.dex.qt.settings.page.hpp"
#include "atomic.dex.qt.utilities.hpp"
#include "atomic.dex.utilities.hpp"

//! Constructo destructor
namespace atomic_dex
{
    settings_page::settings_page(entt::registry& registry, ag::ecs::system_manager& system_manager, std::shared_ptr<QApplication> app, QObject* parent) noexcept
        :
        QObject(parent),
        system(registry), m_system_manager(system_manager), m_app(app)
    {
    }
} // namespace atomic_dex

//! Override
namespace atomic_dex
{
    void
    settings_page::update() noexcept
    {
    }
} // namespace atomic_dex

//! Properties
namespace atomic_dex
{
    QString
    settings_page::get_empty_string() const noexcept
    {
        return m_empty_string;
    }

    QString
    settings_page::get_current_lang() const noexcept
    {
        return QString::fromStdString(m_config.current_lang);
    }

    void
    atomic_dex::settings_page::set_current_lang(QString new_lang) noexcept
    {
        const std::string new_lang_std = new_lang.toStdString();
        change_lang(m_config, new_lang_std);

        auto get_locale = [](const std::string& current_lang) {
            if (current_lang == "tr")
            {
                return QLocale::Language::Turkish;
            }
            if (current_lang == "en")
            {
                return QLocale::Language::English;
            }
            if (current_lang == "fr")
            {
                return QLocale::Language::French;
            }
            return QLocale::Language::AnyLanguage;
        };

        qDebug() << "locale before: " << QLocale().name();
        QLocale::setDefault(get_locale(m_config.current_lang));
        qDebug() << "locale after: " << QLocale().name();
        [[maybe_unused]] auto res = this->m_translator.load("atomic_defi_" + new_lang, QLatin1String(":/atomic_defi_design/assets/languages"));
        assert(res);
        this->m_app->installTranslator(&m_translator);
        this->m_qml_engine->retranslate();
        emit onLangChanged();
        //emit langChanged();
    }

    bool
    atomic_dex::settings_page::is_notification_enabled() const noexcept
    {
        return m_config.notification_enabled;
    }

    void
    settings_page::set_notification_enabled(bool is_enabled) noexcept
    {
        if (m_config.notification_enabled != is_enabled)
        {
            change_notification_status(m_config, is_enabled);
            emit onNotificationEnabledChanged();
        }
    }

    QString
    settings_page::get_current_currency_sign() const noexcept
    {
        return QString::fromStdString(this->m_config.current_currency_sign);
    }

    QString
    settings_page::get_current_fiat_sign() const noexcept
    {
        return QString::fromStdString(this->m_config.current_fiat_sign);
    }

    QString
    settings_page::get_current_currency() const noexcept
    {
        return QString::fromStdString(this->m_config.current_currency);
    }

    void
    settings_page::set_current_currency(const QString& current_currency) noexcept
    {
        if (current_currency.toStdString() != m_config.current_currency)
        {
            spdlog::info("change currency {} to {}", m_config.current_currency, current_currency.toStdString());
            atomic_dex::change_currency(m_config, current_currency.toStdString());
            this->dispatcher_.trigger<update_portfolio_values>();
            emit onCurrencyChanged();
            emit onCurrencySignChanged();
            emit onFiatSignChanged();
        }
    }

    QString
    settings_page::get_current_fiat() const noexcept
    {
        return QString::fromStdString(this->m_config.current_fiat);
    }

    void
    settings_page::set_current_fiat(const QString& current_fiat) noexcept
    {
        if (current_fiat.toStdString() != m_config.current_fiat)
        {
            spdlog::info("change fiat {} to {}", m_config.current_fiat, current_fiat.toStdString());
            atomic_dex::change_fiat(m_config, current_fiat.toStdString());
            emit onFiatChanged();
        }
    }
} // namespace atomic_dex

//! Public API
namespace atomic_dex
{
    atomic_dex::cfg&
    settings_page::get_cfg() noexcept
    {
        return m_config;
    }

    const atomic_dex::cfg&
    settings_page::get_cfg() const noexcept
    {
        return m_config;
    }

    void
    settings_page::init_lang() noexcept
    {
        set_current_lang(QString::fromStdString(m_config.current_lang));
    }
} // namespace atomic_dex

//! QML API
namespace atomic_dex
{
    QStringList
    settings_page::get_available_langs() const
    {
        QStringList out;
        out.reserve(m_config.available_lang.size());
        for (auto&& cur_lang: m_config.available_lang) { out.push_back(QString::fromStdString(cur_lang)); }
        return out;
    }

    QStringList
    settings_page::get_available_fiats() const
    {
        QStringList out;
        out.reserve(m_config.available_fiat.size());
        for (auto&& cur_fiat: m_config.available_fiat) { out.push_back(QString::fromStdString(cur_fiat)); }
        return out;
    }

    QStringList
    settings_page::get_available_currencies() const
    {
        QStringList out;
        out.reserve(m_config.possible_currencies.size());
        for (auto&& cur_currency: m_config.possible_currencies) { out.push_back(QString::fromStdString(cur_currency)); }
        return out;
    }

    bool
    settings_page::is_this_ticker_present_in_raw_cfg(const QString& ticker) const noexcept
    {
        return m_system_manager.get_system<mm2>().is_this_ticker_present_in_raw_cfg(ticker.toStdString());
    }

    bool
    settings_page::is_this_ticker_present_in_normal_cfg(const QString& ticker) const noexcept
    {
        return m_system_manager.get_system<mm2>().is_this_ticker_present_in_normal_cfg(ticker.toStdString());
    }

    QVariantList
    settings_page::get_custom_coins() const noexcept
    {
        auto coins = m_system_manager.get_system<mm2>().get_custom_coins();
        return to_qt_binding(std::move(coins));
    }

    QString
    settings_page::get_custom_coins_icons_path() const noexcept
    {
        return QString::fromStdString(get_runtime_coins_path().string());
    }

    void
    settings_page::process_erc_20_token_add(const QString& contract_address, const QString& coinpaprika_id, const QString& icon_filepath)
    {
        this->set_fetching_erc_data_busy(true);
        using namespace std::string_literals;
        std::string url            = "/api/v1/erc_infos/"s + contract_address.toStdString();
        auto        answer_functor = [this, contract_address, coinpaprika_id, icon_filepath](web::http::http_response resp) {
            //! Extract answer
            std::string    body = TO_STD_STR(resp.extract_string(true).get());
            nlohmann::json out  = nlohmann::json::object();
            out["mm2_cfg"]      = nlohmann::json::object();
            out["adex_cfg"]     = nlohmann::json::object();
            if (resp.status_code() == 200)
            {
                nlohmann::json  body_json = nlohmann::json::parse(body).at("result")[0];
                std::string     ticker    = body_json.at("symbol").get<std::string>();
                const fs::path& suffix    = fs::path(icon_filepath.toStdString()).extension();
                fs::copy_file(
                    icon_filepath.toStdString(),
                    fs::path(get_custom_coins_icons_path().toStdString()) / (boost::algorithm::to_lower_copy(ticker) + suffix.string()),
                    fs::copy_option::overwrite_if_exists);
                if (not is_this_ticker_present_in_raw_cfg(QString::fromStdString(ticker)))
                {
                    out["mm2_cfg"]["protocol"]                              = nlohmann::json::object();
                    out["mm2_cfg"]["protocol"]["type"]                      = "ERC20";
                    out["mm2_cfg"]["protocol"]["protocol_data"]             = nlohmann::json::object();
                    out["mm2_cfg"]["protocol"]["protocol_data"]["platform"] = "ETH";
                    std::string out_address                                 = contract_address.toStdString();
                    boost::algorithm::to_lower(out_address);
                    to_eth_checksum(out_address);
                    out["mm2_cfg"]["protocol"]["protocol_data"]["contract_address"] = out_address;
                    out["mm2_cfg"]["rpc_port"]                                      = 80;
                    out["mm2_cfg"]["coin"]                                          = ticker;
                    out["mm2_cfg"]["mm2"]                                           = 1;
                    out["mm2_cfg"]["decimals"]                                      = std::stoi(body_json.at("divisor").get<std::string>());
                    out["mm2_cfg"]["avg_blocktime"]                                 = 0.25;
                    out["mm2_cfg"]["required_confirmations"]                        = 3;
                    out["mm2_cfg"]["name"]                                          = body_json.at("tokenName").get<std::string>();
                }
                if (not is_this_ticker_present_in_normal_cfg(QString::fromStdString(ticker)))
                {
                    //!
                    out["adex_cfg"][ticker]                   = nlohmann::json::object();
                    out["adex_cfg"][ticker]["coin"]           = ticker;
                    out["adex_cfg"][ticker]["name"]           = body_json.at("tokenName").get<std::string>();
                    out["adex_cfg"][ticker]["coinpaprika_id"] = coinpaprika_id.toStdString();
                    out["adex_cfg"][ticker]["eth_nodes"] =
                        nlohmann::json::array({"http://eth1.cipig.net:8555", "http://eth2.cipig.net:8555", "http://eth3.cipig.net:8555"});
                    out["adex_cfg"][ticker]["explorer_url"]      = nlohmann::json::array({"https://etherscan.io/"});
                    out["adex_cfg"][ticker]["type"]              = "ERC-20";
                    out["adex_cfg"][ticker]["active"]            = false;
                    out["adex_cfg"][ticker]["currently_enabled"] = false;
                    out["adex_cfg"][ticker]["is_custom_coin"]    = true;
                }
            }
            else
            {
                out["error_message"] = body;
                out["error_code"]    = resp.status_code();
            }
            spdlog::trace("result json of fetch erc infos from contract address is: {}", out.dump(4));
            this->set_custom_erc_token_data(nlohmann_json_object_to_qt_json_object(out));
            this->set_fetching_erc_data_busy(false);
        };
        ::mm2::api::async_process_rpc_get("erc_infos", url).then(answer_functor).then(&handle_exception_pplx_task);
    }

    bool
    settings_page::is_fetching_erc_data_busy() const noexcept
    {
        return m_fetching_erc_data_busy.load();
    }

    void
    settings_page::set_fetching_erc_data_busy(bool status) noexcept
    {
        if (m_fetching_erc_data_busy != status)
        {
            m_fetching_erc_data_busy = status;
            emit ercDataStatusChanged();
        }
    }

    QVariant
    settings_page::get_custom_erc_token_data() const noexcept
    {
        return nlohmann_json_object_to_qt_json_object(m_custom_erc_token_data.get());
    }

    void
    settings_page::set_custom_erc_token_data(QVariant rpc_data) noexcept
    {
        nlohmann::json out      = nlohmann::json::parse(QString(QJsonDocument(rpc_data.toJsonObject()).toJson()).toStdString());
        m_custom_erc_token_data = out;
        emit customErcTokenDataChanged();
    }

    void
    settings_page::submit()
    {
        spdlog::trace("submit whole cfg");
        nlohmann::json out = m_custom_erc_token_data.get();
        this->m_system_manager.get_system<mm2>().add_new_coin(out.at("adex_cfg"), out.at("mm2_cfg"));
        this->set_custom_erc_token_data(QJsonObject{{}});
    }

    void
    settings_page::remove_custom_coin(const QString& ticker) noexcept
    {
        spdlog::trace("remove ticker: {}", ticker.toStdString());
        this->m_system_manager.get_system<mm2>().remove_custom_coin(ticker.toStdString());
    }

    void
    settings_page::set_qml_engine(QQmlApplicationEngine* engine) noexcept
    {
        m_qml_engine = engine;
    }
} // namespace atomic_dex