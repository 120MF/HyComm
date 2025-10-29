#include <HyComm/Protocols/SerialTraits.hpp>
#include <HyComm/Configs/SerialConfig.hpp>

hy::ipc::OpenRequest hy::protocols::SerialTraits::make_open_request(const configs::SerialConfig& config)
{
    return ipc::SerialOpenRequest{
        config.device_path,
        config.baud_rate,
        config.data_bits,
        config.stop_bits,
        config.parity,
        config.flow_control,
        config.rts_dtr_on,
        ""
    };
}
