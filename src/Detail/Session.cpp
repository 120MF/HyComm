#include <format>
#include <future>
#include <iostream>
#include <HyComm/Detail/Session.hpp>

#include <HyComm/Common/FdTransfer.hpp>

std::unique_ptr<hy::detail::Session> hy::detail::Session::create()
{
    return std::make_unique<Session>();
}

tl::expected<int, hy::common::Error> hy::detail::Session::open_interface(
    ipc::OpenRequest& open_request)
{
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

    // Step 2: Construct and send request
    auto request = m_client.loan_uninit().expect("");
    ipc::Request payload_value;
    std::visit([&payload_value](const auto& arg)
    {
        payload_value = ipc::Request(arg);
    }, open_request);

    // Step 3: Send request
    auto initialized_request = request.write_payload(std::move(payload_value));
    auto pending_response = iox2::send(std::move(initialized_request)).expect("");

    // Step 4: Notify Daemon
    if (const auto res = m_notifier.notify(); !res)
    {
        return tl::make_unexpected(common::Error{
            common::ErrorCode::ClientNotifyFailed, ""
        });
    }

    // Step 5: Wait for response
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

hy::ipc::Response hy::detail::Session::control_interface(const ipc::ConfigRequest& control_request)
{
    auto request = m_client.loan_uninit().expect("");
    ipc::Request payload_value;
    std::visit([&payload_value](const auto& arg)
    {
        payload_value = ipc::Request(arg);
    }, control_request);
    auto initialized_request = request.write_payload(std::move(payload_value));
    auto pending_response = iox2::send(std::move(initialized_request)).expect("");

    if (const auto res = m_notifier.notify(); !res)
    {
        return tl::make_unexpected(common::Error{
            common::ErrorCode::ClientNotifyFailed, ""
        });
    }

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
            return res->payload();
        }
    }
    // Timeout: no response received
    return tl::make_unexpected(common::Error{
        common::ErrorCode::ClientReceiveFailed, "Timeout waiting for response from server"
    });
}

hy::ipc::Response hy::detail::Session::close_interface(const ipc::CloseRequest& close_request)
{
    auto request = m_client.loan_uninit().expect("");
    ipc::Request payload_value;
    std::visit([&payload_value](const auto& arg)
    {
        payload_value = ipc::Request(arg);
    }, close_request);
    auto initialized_request = request.write_payload(std::move(payload_value));
    auto pending_response = iox2::send(std::move(initialized_request)).expect("");

    if (const auto res = m_notifier.notify(); !res)
    {
        return tl::make_unexpected(common::Error{
            common::ErrorCode::ClientNotifyFailed, ""
        });
    }

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
            return res->payload();
        }
    }
    // Timeout: no response received
    return tl::make_unexpected(common::Error{
        common::ErrorCode::ClientReceiveFailed, "Timeout waiting for response from server"
    });
}

hy::detail::Session::Session() :
    m_node([]
    {
        std::cout << "Session: Creating IPC node..." << std::endl;
        std::cout.flush();
        auto result = iox2::NodeBuilder().create<iox2::ServiceType::Ipc>();
        std::cout << "Session: Node result has_value = " << result.has_value() << std::endl;
        std::cout.flush();
        return std::move(result.expect("Failed to create IPC node"));
    }()),
    m_notifier([this]
    {
        std::cout << "Session: Creating notifier..." << std::endl;
        std::cout.flush();
        auto svc_name = iox2::ServiceName::create("HyCommDaemonEvent");
        std::cout << "Session: Service name result has_value = " << svc_name.has_value() << std::endl;
        std::cout.flush();
        auto name = svc_name.expect("Failed to create event service name");
        auto svc = m_node.service_builder(name)
                         .event().open_or_create();
        std::cout << "Session: Event service result has_value = " << svc.has_value() << std::endl;
        std::cout.flush();
        auto service = std::move(svc.expect("Failed to open or create event service"));
        auto notifier = service.notifier_builder().create();
        std::cout << "Session: Notifier result has_value = " << notifier.has_value() << std::endl;
        std::cout.flush();
        return std::move(notifier.expect("Failed to create notifier"));
    }()),
    m_client([this]
    {
        std::cout << "Session: Creating client..." << std::endl;
        std::cout.flush();
        auto svc_name = iox2::ServiceName::create("HyCommDaemonServer");
        std::cout << "Session: Server service name result has_value = " << svc_name.has_value() << std::endl;
        std::cout.flush();
        auto name = svc_name.expect("Failed to create server service name");
        auto svc = m_node.service_builder(name)
                         .request_response<ipc::Request, ipc::Response>()
                         .open_or_create();
        std::cout << "Session: RR service result has_value = " << svc.has_value() << std::endl;
        std::cout.flush();
        auto service = std::move(svc.expect("Failed to open or create request-response service"));
        auto client = service.client_builder().create();
        std::cout << "Session: Client result has_value = " << client.has_value() << std::endl;
        std::cout.flush();
        return std::move(client.expect("Failed to create client"));
    }())
{
    std::cout << "Session: Constructor completed!" << std::endl;
    std::cout.flush();
}
