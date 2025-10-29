#ifndef HYCOMM_SERIALCONFIG_HPP
#define HYCOMM_SERIALCONFIG_HPP

#include <string>
#include <HyComm/IpcShared/SerialRequest.hpp>

namespace hy::configs
{
    struct SerialConfig
    {
        std::string device_path = "/dev/ttyUSB0";
        uint32_t baud_rate = 115200;
        ipc::DataBits data_bits = ipc::DataBits::BITS_8;
        ipc::StopBits stop_bits = ipc::StopBits::ONE;
        ipc::Parity parity = ipc::Parity::NONE;
        ipc::FlowControl flow_control = ipc::FlowControl::NONE;
        bool rts_dtr_on = false;
    };
}

namespace hy
{
    using SerialConfig = configs::SerialConfig;
}

#endif //HYCOMM_SERIALCONFIG_HPP
