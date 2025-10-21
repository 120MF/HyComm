#ifndef HYCOMM_SERIALREQUEST_HPP
#define HYCOMM_SERIALREQUEST_HPP

namespace hy::ipc
{
    // 数据位枚举
    enum class DataBits : uint8_t
    {
        BITS_5 = 5,
        BITS_6 = 6,
        BITS_7 = 7,
        BITS_8 = 8,
    };

    // 停止位枚举
    enum class StopBits : uint8_t
    {
        ONE = 1,
        TWO = 2,
    };

    // 校验位枚举
    enum class Parity : uint8_t
    {
        NONE = 0, // 无校验
        ODD = 1, // 奇校验
        EVEN = 2, // 偶校验
    };

    // 流控制枚举
    enum class FlowControl : uint8_t
    {
        NONE = 0, // 无
        RTS_CTS = 1, // RTS/CTS硬件流控
        XON_XOFF = 2, // XON/XOFF软件流控
    };

    struct SerialOpenRequest
    {
        std::string interface_name; // "/dev/ttyUSB0"
        uint32_t baud_rate; // 115200
        DataBits data_bits; // BITS_8
        StopBits stop_bits; // ONE
        Parity parity; // NONE
        FlowControl flow_control; // NONE
        bool rts_dtr_on; // 是否启用RTS/DTR (某些设备需要)
        std::string socket_path; // Client监听的UDS路径
    };

    struct SerialConfigRequest
    {
        std::string interface_name;
        DataBits data_bits;
        StopBits stop_bits;
        Parity parity;
        FlowControl flow_control;
    };

    struct SerialCloseRequest
    {
        std::string interface_name;
    };
}

#endif //HYCOMM_SERIALREQUEST_HPP
