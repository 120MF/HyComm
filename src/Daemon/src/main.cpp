#include <iox2/iceoryx2.hpp>
#include <HyComm/IpcShared/Payload.hpp>
constexpr iox::units::Duration CYCLE_TIME = iox::units::Duration::fromSeconds(1);

int main()
{
    using namespace iox2;
    set_log_level_from_env_or(LogLevel::Info);
    const auto node = NodeBuilder().create<ServiceType::Ipc>().expect("node");
    const auto service = node.service_builder(ServiceName::create("HyCommDaemon").expect("name"))
                             .request_response<hy::ipc::Request, hy::ipc::Response>()
                             .open_or_create()
                             .expect("service");
    auto server = service.server_builder().create().expect("");
    std::cout << "Listener ready to receive events!\n";

    return 0;
}
