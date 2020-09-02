#pragma once

//! QT Headers
#include <QJsonObject>
#include <QObject>
#include <QVariant>

namespace atomic_dex
{
    class wallet_page final : public QObject, public ag::ecs::pre_update_system<wallet_page>
    {
        //! Q_Object definition
        Q_OBJECT

        //! Properties
        Q_PROPERTY(QString ticker READ get_current_ticker WRITE set_current_ticker NOTIFY currentTickerChanged)
        Q_PROPERTY(QVariant ticker_infos READ get_ticker_infos NOTIFY tickerInfosChanged)
        Q_PROPERTY(bool is_claiming_rpc_ready READ is_rpc_claiming_ready WRITE set_rpc_claiming_ready NOTIFY rpcClaimingStatusChanged)
        Q_PROPERTY(QVariant claiming_rpc_data READ get_rpc_claiming_data WRITE set_rpc_claiming_data NOTIFY claimingRpcDataChanged)

        using t_qt_synchronized_json = boost::synchronized_value<QJsonObject>;

        //! Private fields
        ag::ecs::system_manager& m_system_manager;
        std::atomic_bool         m_is_claiming_rpc_ready{false};
        t_qt_synchronized_json   m_claiming_rpc_result;

      public:
        explicit wallet_page(entt::registry& registry, ag::ecs::system_manager& system_manager, QObject* parent = nullptr);
        void update() noexcept override;
        ~wallet_page() noexcept final = default;

        //! Public API
        void             refresh_ticker_infos() noexcept;
        Q_INVOKABLE void claim_rewards();

        //! Properties
        [[nodiscard]] QString  get_current_ticker() const noexcept;
        void                   set_current_ticker(const QString& ticker) noexcept;
        [[nodiscard]] QVariant get_ticker_infos() const noexcept;
        [[nodiscard]] bool     is_rpc_claiming_ready() const noexcept;
        void                   set_rpc_claiming_ready(bool status) noexcept;
        [[nodiscard]] QVariant get_rpc_claiming_data() const noexcept;
        void                   set_rpc_claiming_data(QVariant rpc_data) noexcept;;

      signals:
        void currentTickerChanged();
        void tickerInfosChanged();
        void rpcClaimingStatusChanged();
        void claimingRpcDataChanged();
    };
} // namespace atomic_dex

REFL_AUTO(type(atomic_dex::wallet_page))