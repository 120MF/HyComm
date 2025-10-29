#ifndef HYCOMM_ECHOBACKEND_HPP
#define HYCOMM_ECHOBACKEND_HPP

#include <ankerl/unordered_dense.h>
#include <mutex>
#include "IInterfaceBackend.hpp"

namespace hy::daemon
{
    class EchoBackend final : public IInterfaceBackend
    {
    public:
        EchoBackend();
        ipc::Response handle_request(const ipc::Request& req) override;

    private:
        ipc::Response handle_open(const ipc::EchoOpenRequest& req);
        ipc::Response handle_config(const ipc::EchoConfigRequest& req);
        ipc::Response handle_close(const ipc::EchoCloseRequest& req);

        struct EchoInstance
        {
            int fd_pair[2]; // socketpair for bidirectional communication
            bool uppercase_mode = false;
        };

        ankerl::unordered_dense::map<std::string, EchoInstance> m_instances;
        std::mutex m_mutex;
    };
}

#endif //HYCOMM_ECHOBACKEND_HPP

