#ifndef HYCOMM_MANAGER_HPP
#define HYCOMM_MANAGER_HPP
#include <memory>

#include "Detail/Session.hpp"
#include "Detail/GenericInterface.hpp"
#include "IpcShared/Request.hpp"

namespace hy
{
    class Manager
    {
    public:
        static std::unique_ptr<Manager> create() { return {}; };

        template <typename ConcreteInterface, typename... Args>
        tl::expected<std::shared_ptr<ConcreteInterface>, common::Error> get_interface(Args&&... args)
        {
            using Traits = typename ConcreteInterface::Traits;

            auto req = Traits::make_open_request(std::forward<Args>(args)...);
            auto fd_res = m_session->open_interface(req);
            if (!fd_res)
            {
                return tl::make_unexpected(fd_res.error());
            }
            int fd = fd_res.value();

            auto interface = std::make_shared<ConcreteInterface>(fd, m_uring_manager);

            return interface;
        }

    private:
        Manager() : m_session(detail::Session::create()), m_uring_manager(detail::UringManager::create())
        {
        }

        std::unique_ptr<detail::Session> m_session;
        std::shared_ptr<detail::UringManager> m_uring_manager;
    };
}

#endif //HYCOMM_MANAGER_HPP
