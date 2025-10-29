#include <iostream>
#include <iox2/iceoryx2.hpp>
#include <HyComm/IpcShared/Request.hpp>
#include <HyComm/IpcShared/Response.hpp>

int main()
{
    std::cout << "Testing actual Request/Response types with iceoryx2..." << std::endl;

    std::cout << "Is ipc::Request trivially copyable? "
        << std::is_trivially_copyable_v<hy::ipc::Request> << std::endl;
    std::cout << "Is ipc::Response trivially copyable? "
        << std::is_trivially_copyable_v<hy::ipc::Response> << std::endl;

    std::cout << "Creating node..." << std::endl;
    auto node_result = iox2::NodeBuilder().create<iox2::ServiceType::Ipc>();
    if (!node_result.has_value())
    {
        std::cerr << "Failed to create node!" << std::endl;
        return 1;
    }
    auto node = std::move(node_result.value());
    std::cout << "Node created successfully" << std::endl;

    std::cout << "Creating service name..." << std::endl;
    auto service_name_result = iox2::ServiceName::create("TestActualTypes");
    if (!service_name_result.has_value())
    {
        std::cerr << "Failed to create service name!" << std::endl;
        return 1;
    }
    auto service_name = std::move(service_name_result.value());
    std::cout << "Service name created successfully" << std::endl;

    std::cout << "Opening/creating service with actual Request/Response..." << std::endl;
    auto service_result = node.service_builder(service_name)
                              .request_response<hy::ipc::Request, hy::ipc::Response>()
                              .open_or_create();
    if (!service_result.has_value())
    {
        std::cerr << "Failed to open/create service with actual types!" << std::endl;
        return 1;
    }
    std::cout << "Service opened/created successfully with actual types!" << std::endl;
    auto service = std::move(service_result.value());

    std::cout << "Creating client..." << std::endl;
    auto client_result = service.client_builder().create();
    if (!client_result.has_value())
    {
        std::cerr << "Failed to create client!" << std::endl;
        return 1;
    }
    std::cout << "Client created successfully" << std::endl;

    std::cout << "\n=== All tests passed! Actual Request/Response types work with iceoryx2 ===" << std::endl;
    return 0;
}

