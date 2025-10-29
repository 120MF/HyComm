#include <HyComm/Manager.hpp>
#include <HyComm/Interfaces/SerialInterface.hpp>
#include <HyComm/IpcShared/SerialRequest.hpp>
#include <iostream>

int main()
{
    try
    {
        auto manager = hy::Manager::create();

        // Method 1: Using Builder pattern (Recommended)
        auto serial = manager->serial()
            .device("/dev/ttyUSB0")
            .baud_rate(115200)
            .data_bits(hy::ipc::DataBits::BITS_8)
            .parity(hy::ipc::Parity::NONE)
            .build();

        // Open the interface
        if (auto res = serial.open(); !res)
        {
            std::cerr << "Failed to open serial: " << res.error().message << std::endl;
            return 1;
        }

        // Set receive callback
        serial->set_callback([](const std::vector<uint8_t>& frame)
        {
            std::cout << "Received " << frame.size() << " bytes" << std::endl;
        });

        // Send data
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        // serial->send(data, callback);

        // Reconfigure at runtime
        auto new_config = serial.get_config();
        new_config.baud_rate = 9600;
        if (auto res = serial.reconfigure(new_config); !res)
        {
            std::cerr << "Failed to reconfigure: " << res.error().message << std::endl;
        }

        // Close and reopen
        serial.close();
        serial.open();

        // Method 2: Using configuration object
        hy::SerialConfig config;
        config.device_path = "/dev/ttyUSB1";
        config.baud_rate = 57600;

        auto serial2 = manager->create_interface<hy::Serial>(config);
        serial2.open();

        std::cout << "Serial interfaces created successfully!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
