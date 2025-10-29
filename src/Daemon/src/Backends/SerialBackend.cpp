#include "SerialBackend.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/ioctl.h>
#include <cerrno>
#include <format>

#include "HyComm/Common/FdTransfer.hpp"

hy::daemon::SerialBackend::SerialBackend()
{
}

hy::ipc::Response hy::daemon::SerialBackend::handle_request(const ipc::Request& req)
{
    if (auto* open_req = std::get_if<ipc::SerialOpenRequest>(&req))
    {
        return handle_open(*open_req);
    }

    if (auto* config_req = std::get_if<ipc::SerialConfigRequest>(&req))
    {
        return handle_config(*config_req);
    }

    if (auto* close_req = std::get_if<ipc::SerialCloseRequest>(&req))
    {
        return handle_close(*close_req);
    }

    return tl::make_unexpected(common::Error{
        common::ErrorCode::InvalidArguments,
        "Unknown request type in SerialBackend"
    });
}

hy::ipc::Response hy::daemon::SerialBackend::handle_open(const ipc::SerialOpenRequest& req)
{
    std::lock_guard lock(m_device_mutex);
    auto it = m_open_devices.find(req.interface_name);
    if (it != m_open_devices.end())
    {
        return tl::make_unexpected(common::Error{
            common::ErrorCode::AlreadyOpen, std::format("Device already open: {}", req.interface_name)
        });
    }

    const int fd = open_serial_port(req);
    if (fd < 0)
    {
        return tl::make_unexpected(common::Error{
            common::ErrorCode::InterfaceNotFound,
            std::format("Failed to open serial port: {}, {}", req.interface_name, strerror(errno))
        });
    }

    if (!configure_serial_port(fd, req))
    {
        close(fd);
        return tl::make_unexpected(common::Error{
            common::ErrorCode::InvalidArguments,
            std::format("Failed to configure serial port: {}, {}", req.interface_name, strerror(errno))
        });
    }

    if (!common::FdTransfer::connect_and_send(req.uds_path, fd))
    {
        close(fd);
        return tl::make_unexpected(common::Error{
            common::ErrorCode::FdTransferFailed,
            std::format("Failed to send FD via UDS to: {}, {}", req.uds_path, strerror(errno))
        });
    }
    m_open_devices[req.interface_name] = fd;
    return ipc::GenericSuccessResponse{};
}

hy::ipc::Response hy::daemon::SerialBackend::handle_config(const ipc::SerialConfigRequest& req)
{
    std::lock_guard lock(m_device_mutex);
    auto it = m_open_devices.find(req.interface_name);
    if (it == m_open_devices.end())
    {
        return tl::make_unexpected(common::Error{
            common::ErrorCode::InterfaceNotFound,
            std::format("Device not open: {}", req.interface_name)
        });
    }
    const int fd = it->second;
    termios tios;
    if (tcgetattr(fd, &tios) < 0)
    {
        return tl::make_unexpected(common::Error{
            common::ErrorCode::InvalidArguments,
            std::format("Failed to read device configuration: {}", strerror(errno))
        });
    }

    // 应用新的配置
    tios.c_cflag &= ~CSIZE;
    tios.c_cflag |= data_bits_to_cflag(req.data_bits);

    tios.c_cflag &= ~CSTOPB;
    tios.c_cflag |= stop_bits_to_cflag(req.stop_bits);

    apply_parity(tios, req.parity);
    apply_flow_control(tios, req.flow_control);

    if (tcsetattr(fd, TCSANOW, &tios) < 0)
    {
        return tl::make_unexpected(common::Error{
            common::ErrorCode::InvalidArguments,
            std::format("Failed to apply configuration: {}", strerror(errno))
        });
    }

    return ipc::GenericSuccessResponse{};
}

hy::ipc::Response hy::daemon::SerialBackend::handle_close(const ipc::SerialCloseRequest& req)
{
    std::lock_guard lock(m_device_mutex);
    const auto it = m_open_devices.find(req.interface_name);
    if (it == m_open_devices.end())
    {
        return tl::make_unexpected(common::Error{
            common::ErrorCode::InterfaceNotFound,
            std::format("Device not open: {}", req.interface_name)
        });
    }

    close(it->second);
    m_open_devices.erase(it);

    return ipc::GenericSuccessResponse{};
}

int hy::daemon::SerialBackend::open_serial_port(const ipc::SerialOpenRequest& req)
{
    const int fd = open(req.interface_name.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        return -1;
    }
    return fd;
}

bool hy::daemon::SerialBackend::configure_serial_port(const int fd, const ipc::SerialOpenRequest& req)
{
    termios tios;
    if (tcgetattr(fd, &tios) < 0)
    {
        return false;
    }
    const tcflag_t baud_cflag = baud_to_cflag(req.baud_rate);
    if (cfsetispeed(&tios, baud_cflag) < 0 || cfsetospeed(&tios, baud_cflag) < 0)
    {
        return false;
    }
    // 设置数据位
    tios.c_cflag &= ~CSIZE; // 清除数据位标志
    tios.c_cflag |= data_bits_to_cflag(req.data_bits);

    // 设置停止位
    tios.c_cflag &= ~CSTOPB;
    tios.c_cflag |= stop_bits_to_cflag(req.stop_bits);

    // 设置校验位
    apply_parity(tios, req.parity);

    // 设置流控制
    apply_flow_control(tios, req.flow_control);

    // 基础配置
    tios.c_cflag |= (CREAD | CLOCAL); // 启用接收,忽略调制解调器信号

    // 禁用规范模式
    tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // 禁用特殊字符处理
    tios.c_oflag &= ~OPOST;

    // 设置超时
    tios.c_cc[VTIME] = 0;
    tios.c_cc[VMIN] = 0;

    // 应用配置
    if (tcsetattr(fd, TCSANOW, &tios) < 0)
    {
        return false;
    }

    // 设置RTS/DTR
    if (req.rts_dtr_on)
    {
        int status;
        if (ioctl(fd, TIOCMGET, &status) < 0)
        {
            return false;
        }
        status |= TIOCM_RTS | TIOCM_DTR;
        if (ioctl(fd, TIOCMSET, &status) < 0)
        {
            return false;
        }
    }

    return true;
}

tcflag_t hy::daemon::SerialBackend::baud_to_cflag(uint32_t baud)
{
    switch (baud)
    {
    case 110: return B110;
    case 300: return B300;
    case 600: return B600;
    case 1200: return B1200;
    case 2400: return B2400;
    case 4800: return B4800;
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
    case 460800: return B460800;
    default: return B115200;
    }
}

tcflag_t hy::daemon::SerialBackend::data_bits_to_cflag(ipc::DataBits bits)
{
    switch (bits)
    {
    case ipc::DataBits::BITS_5: return CS5;
    case ipc::DataBits::BITS_6: return CS6;
    case ipc::DataBits::BITS_7: return CS7;
    case ipc::DataBits::BITS_8: return CS8;
    default: return CS8;
    }
}

tcflag_t hy::daemon::SerialBackend::stop_bits_to_cflag(ipc::StopBits bits)
{
    switch (bits)
    {
    case ipc::StopBits::ONE: return 0; // CSTOPB = 0
    case ipc::StopBits::TWO: return CSTOPB;
    default: return 0;
    }
}

void hy::daemon::SerialBackend::apply_parity(termios& tios, const ipc::Parity parity)
{
    switch (parity)
    {
    case ipc::Parity::NONE:
        tios.c_cflag &= ~PARENB;
        break;
    case ipc::Parity::ODD:
        tios.c_cflag |= PARENB | PARODD;
        break;
    case ipc::Parity::EVEN:
        tios.c_cflag |= PARENB;
        tios.c_cflag &= ~PARODD;
        break;
    }
}

void hy::daemon::SerialBackend::apply_flow_control(termios& tios, const ipc::FlowControl flow_control)
{
    switch (flow_control)
    {
    case ipc::FlowControl::NONE:
        tios.c_cflag &= ~CRTSCTS;
        tios.c_iflag &= ~(IXON | IXOFF | IXANY);
        break;
    case ipc::FlowControl::RTS_CTS:
        tios.c_cflag |= CRTSCTS;
        tios.c_iflag &= ~(IXON | IXOFF | IXANY);
        break;
    case ipc::FlowControl::XON_XOFF:
        tios.c_cflag &= ~CRTSCTS;
        tios.c_iflag |= IXON | IXOFF | IXANY;
        break;
    }
}
