#ifndef HYCOMM_INTERFACEHANDLE_HPP
#define HYCOMM_INTERFACEHANDLE_HPP

#include <memory>
#include <tl/expected.hpp>

#include "HyComm/Common/Error.hpp"
#include "HyComm/Detail/InterfaceState.hpp"
#include "HyComm/Detail/Session.hpp"

namespace hy
{
    template <typename ConcreteInterface>
    class InterfaceHandle
    {
    public:
        using ConfigType = typename ConcreteInterface::ConfigType;
        using Traits = typename ConcreteInterface::Traits;

        InterfaceHandle(std::shared_ptr<ConcreteInterface> interface,
                       std::shared_ptr<detail::Session> session,
                       ConfigType config)
            : m_interface(std::move(interface))
            , m_session(std::move(session))
            , m_config(std::move(config))
            , m_state(detail::InterfaceState::Closed)
        {
        }

        tl::expected<void, common::Error> open()
        {
            if (m_state == detail::InterfaceState::Open)
            {
                return tl::make_unexpected(common::Error{
                    common::ErrorCode::AlreadyOpen, "Interface already open"
                });
            }

            m_state = detail::InterfaceState::Opening;
            
            auto result = m_interface->open_internal();
            if (!result)
            {
                m_state = detail::InterfaceState::Error;
                return result;
            }

            m_state = detail::InterfaceState::Open;
            return {};
        }

        tl::expected<void, common::Error> close()
        {
            if (m_state != detail::InterfaceState::Open)
            {
                return tl::make_unexpected(common::Error{
                    common::ErrorCode::InterfaceNotFound, "Interface not open"
                });
            }

            m_state = detail::InterfaceState::Closing;

            auto close_req = Traits::make_close_request(m_config);
            auto result = m_session->close_interface(close_req);
            
            if (!result)
            {
                m_state = detail::InterfaceState::Error;
                return tl::make_unexpected(result.error());
            }

            m_interface->close_internal();
            m_state = detail::InterfaceState::Closed;
            return {};
        }

        tl::expected<void, common::Error> reconfigure(const ConfigType& new_config)
        {
            if (m_state != detail::InterfaceState::Open)
            {
                return tl::make_unexpected(common::Error{
                    common::ErrorCode::InterfaceNotFound, "Interface must be open to reconfigure"
                });
            }

            auto config_req = Traits::make_config_request(new_config);
            auto result = m_session->control_interface(config_req);
            
            if (!result)
            {
                return tl::make_unexpected(result.error());
            }

            m_config = new_config;
            return {};
        }

        bool is_open() const { return m_state == detail::InterfaceState::Open; }
        
        detail::InterfaceState get_state() const { return m_state; }
        
        const ConfigType& get_config() const { return m_config; }

        ConcreteInterface* operator->()
        {
            return m_interface.get();
        }

        const ConcreteInterface* operator->() const
        {
            return m_interface.get();
        }

        ConcreteInterface& operator*()
        {
            return *m_interface;
        }

        const ConcreteInterface& operator*() const
        {
            return *m_interface;
        }

    private:
        std::shared_ptr<ConcreteInterface> m_interface;
        std::shared_ptr<detail::Session> m_session;
        ConfigType m_config;
        detail::InterfaceState m_state;
    };
}

#endif //HYCOMM_INTERFACEHANDLE_HPP
