#ifndef HYCOMM_MANAGER_HPP
#define HYCOMM_MANAGER_HPP
#include <memory>
#include <iostream>

#include "Detail/Session.hpp"
#include "Detail/GenericInterface.hpp"
#include "Detail/InterfaceHandle.hpp"
#include "IpcShared/Request.hpp"
#include "Builders/SerialBuilder.hpp"
#include "Configs/SerialConfig.hpp"

namespace hy
{
    class Manager
    {
    public:
        static std::unique_ptr<Manager> create()
        {
            return std::unique_ptr<Manager>(new Manager());
        }

        SerialBuilder serial()
        {
            return SerialBuilder(this);
        }

        template <typename ConcreteInterface>
        InterfaceHandle<ConcreteInterface> create_interface(const typename ConcreteInterface::ConfigType& config)
        {
            std::cout << "Manager::create_interface: Starting..." << std::endl;
            std::cout.flush();

            using Traits = typename ConcreteInterface::Traits;

            std::cout << "Manager::create_interface: Calling make_open_request..." << std::endl;
            std::cout.flush();

            auto req = Traits::make_open_request(config);

            std::cout << "Manager::create_interface: Calling open_interface..." << std::endl;
            std::cout.flush();

            auto fd_res = m_session->open_interface(req);

            std::cout << "Manager::create_interface: open_interface returned, has_value = " << fd_res.has_value() <<
                std::endl;
            std::cout.flush();
            if (!fd_res)
            {
                throw std::runtime_error("Failed to open interface: " + fd_res.error().message);
            }
            int fd = fd_res.value();

            std::cout << "Manager::create_interface: Creating shared_ptr to interface, fd = " << fd << std::endl;
            std::cout.flush();

            auto interface = std::make_shared<ConcreteInterface>(
                typename ConcreteInterface::PrivateTag{}, fd, m_uring_manager);

            std::cout << "Manager::create_interface: Creating InterfaceHandle..." << std::endl;
            std::cout.flush();

            return InterfaceHandle<ConcreteInterface>(interface, m_session, config);
        }

        std::shared_ptr<detail::Session> get_session() { return m_session; }
        std::shared_ptr<detail::UringManager> get_uring_manager() { return m_uring_manager; }

    private:
        Manager() : m_session([]
                    {
                        std::cout << "Manager: Creating Session..." << std::endl;
                        std::cout.flush();
                        auto session = detail::Session::create();
                        std::cout << "Manager: Session created" << std::endl;
                        std::cout.flush();
                        return session;
                    }()),
                    m_uring_manager([]
                    {
                        std::cout << "Manager: Creating UringManager..." << std::endl;
                        std::cout.flush();
                        auto uring = detail::UringManager::create();
                        std::cout << "Manager: UringManager created" << std::endl;
                        std::cout.flush();
                        return uring;
                    }())
        {
            std::cout << "Manager: Constructor body entered" << std::endl;
            std::cout.flush();
        }

        std::shared_ptr<detail::Session> m_session;
        std::shared_ptr<detail::UringManager> m_uring_manager;
    };
}

#endif //HYCOMM_MANAGER_HPP
