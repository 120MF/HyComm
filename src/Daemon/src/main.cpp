#include "DaemonNode.hpp"
#include "InterfaceManager.hpp"

int main()
{
    using namespace hy::daemon;
    auto manager = std::make_shared<InterfaceManager>();
    DaemonNode node([manager](const hy::ipc::Request& req)
    {
        return manager->handle_request(req);
    });
    node.run();
    return 0;
}
