#include <format>
#include <future>
#include <HyComm/Detail/Session.hpp>

#include "HyComm/Common/FdTransfer.hpp"
#include "HyComm/IpcShared/Concepts.hpp"

std::unique_ptr<hy::detail::Session> hy::detail::Session::create()
{
    return std::make_unique<Session>();
}

tl::expected<std::optional<int>, hy::common::Error> hy::detail::Session::open_interface(
    const ipc::Request& open_request)
{
    auto request = m_client.loan_uninit().expect("");
    auto payload = request.payload<>();
    payload = open_request;
    std::future<tl::expected<int, common::Error>> uds_fut;

    std::visit([&]<typename T0>(T0& arg)
    {
        using T = std::decay_t<T0>();
        if constexpr (RequestWithUdsPath<T>)
        {
            auto res = common::FdTransfer::create_listener();
            if (!res)
            {
                return tl::make_unexpected(res.error());
            }
            auto transfer = std::move(*res);
            auto uds_path = transfer.get_socket_path();
            arg.uds_path = uds_path;
            uds_fut = std::async([&]
            {
                return transfer.accept_and_receive();
            });
        }
    }, payload);
    auto initialized_request = request.write_payload(std::move(payload));
    auto pending_response = iox2::send(std::move(initialized_request)).expect("");

    while (m_node.wait(iox::units::Duration::zero()).has_value())
    {
        while (true)
        {
            auto res = pending_response.receive().expect("");
            if (!pending_response.is_connected())
            {
                return tl::make_unexpected(common::Error{
                    common::ErrorCode::ClientReceiveFailed, std::format("Failed to receive message from server.")
                });
            }
            if (!res.has_value())
            {
                continue;
            }
        }
    }
}

hy::ipc::Response hy::detail::Session::control_interface(const ipc::Request& control_request)
{
}

hy::detail::Session::Session() :
    m_node([]
    {
        return iox2::NodeBuilder().create<iox2::ServiceType::Ipc>().expect("");
    }()),
    m_notifier([this]
    {
        return m_node.service_builder(iox2::ServiceName::create("HyCommDaemonEvent").expect(""))
                     .event().open_or_create().expect("")
                     .notifier_builder().create().expect("");
    }()),
    m_client([this]
    {
        return m_node.service_builder(iox2::ServiceName::create("HyCommDaemonService").expect(""))
                     .request_response<ipc::Request, ipc::Response>()
                     .open_or_create().expect("")
                     .client_builder().create().expect("");
    }())
{
}
