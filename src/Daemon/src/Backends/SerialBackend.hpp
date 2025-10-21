#ifndef HYCOMM_SERIALBACKEND_HPP
#define HYCOMM_SERIALBACKEND_HPP
#include <termios.h>
#include <ankerl/unordered_dense.h>

#include "IInterfaceBackend.hpp"

namespace hy::daemon
{
    class SerialBackend final : public IInterfaceBackend
    {
    public:
        SerialBackend();
        ipc::Response handle_request(const ipc::Request& req) override;

    private:
        ipc::Response handle_open(const ipc::SerialOpenRequest& req);
        ipc::Response handle_config(const ipc::SerialConfigRequest& req);
        ipc::Response handle_close(const ipc::SerialCloseRequest& req);

        static int open_serial_port(const ipc::SerialOpenRequest& req);
        static bool configure_serial_port(int fd, const ipc::SerialOpenRequest& req);
        static tcflag_t baud_to_cflag(uint32_t baud);
        static tcflag_t data_bits_to_cflag(ipc::DataBits bits);
        static tcflag_t stop_bits_to_cflag(ipc::StopBits bits);
        static void apply_parity(termios& tios, ipc::Parity parity);
        static void apply_flow_control(termios& tios, ipc::FlowControl flow_control);

        ankerl::unordered_dense::map<std::string, int> m_open_devices;
        std::mutex m_device_mutex;
    };
}


#endif //HYCOMM_SERIALBACKEND_HPP
