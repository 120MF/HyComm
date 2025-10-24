#include <format>
#include <future>
#include <HyComm/Detail/Session.hpp>

#include <HyComm/Common/FdTransfer.hpp>

std::unique_ptr<hy::detail::Session> hy::detail::Session::create()
{
    return {};
}

tl::expected<int, hy::common::Error> hy::detail::Session::open_interface(
    ipc::OpenRequest& open_request)
{
    auto request = m_client.loan_uninit().expect("");
    auto payload = request.payload<>();

    std::future<tl::expected<int, common::Error>> uds_fut;
    bool listener_created = false;

    // Step 1: Create UDS listener and set uds_path
    std::visit([&](auto& arg)
    {
        auto res = common::FdTransfer::create_listener();
        if (!res)
        {
            return;
        }
        auto transfer = std::move(*res);
        auto uds_path = transfer.get_socket_path();
        arg.uds_path = uds_path;
        uds_fut = std::async([transfer = std::move(transfer)]() mutable
        {
            return transfer.accept_and_receive();
        });
        listener_created = true;
    }, open_request);

    if (!listener_created)
    {
        return tl::make_unexpected(common::Error{
            common::ErrorCode::ResourceCreationFailed, "Failed to create UDS listener"
        });
    }

    // Step 2: Construct Request payload
    std::visit([&payload](const auto& arg)
    {
        payload = ipc::Request(arg);
    }, open_request);

    // Step 3: Send request
    auto initialized_request = request.write_payload(std::move(payload));
    auto pending_response = iox2::send(std::move(initialized_request)).expect("");

    // Step 4: Wait for response
    while (m_node.wait(iox::units::Duration::zero()).has_value())
    {
        auto res = pending_response.receive().expect("");
        if (!pending_response.is_connected())
        {
            return tl::make_unexpected(common::Error{
                common::ErrorCode::ClientReceiveFailed, "Failed to receive message from server"
            });
        }
        if (res.has_value())
        {
            // Response received successfully, return the uds_fut result
            return uds_fut.get();
        }
    }
    // Timeout: no response received
    return tl::make_unexpected(common::Error{
        common::ErrorCode::ClientReceiveFailed, "Timeout waiting for response from server"
    });
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
