#ifndef HYCOMM_SESSION_HPP
#define HYCOMM_SESSION_HPP
#include <memory>
#include <optional>
#include <tl/expected.hpp>
#include <iox2/iceoryx2.hpp>

#include "HyComm/Common/Error.hpp"
#include "HyComm/IpcShared/Request.hpp"
#include "HyComm/IpcShared/Response.hpp"

namespace hy::detail
{
    class Session
    {
    public:
        static std::unique_ptr<Session> create();

        tl::expected<std::optional<int>, common::Error> open_interface(const ipc::Request& open_request);

        ipc::Response control_interface(const ipc::Request& control_request);

    private:
        Session();
        iox2::Node<iox2::ServiceType::Ipc> m_node;
        iox2::Notifier<iox2::ServiceType::Ipc> m_notifier;
        iox2::Client<iox2::ServiceType::Ipc, ipc::Request, void, ipc::Response, void> m_client;
    };
}

#endif //HYCOMM_SESSION_HPP
