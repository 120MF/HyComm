#include <HyComm/Manager.hpp>
#include <HyComm/Interfaces/EchoInterface.hpp>
#include <HyComm/Configs/EchoConfig.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

namespace hy
{
    using EchoConfig = configs::EchoConfig;
}

// Simple test for Echo interface
int main()
{
    try
    {
        std::cout << "Starting Echo Interface Test..." << std::endl;
        std::cout << "Creating Manager..." << std::endl;
        std::cout.flush();

        auto manager = hy::Manager::create();

        std::cout << "Manager created successfully!" << std::endl;
        std::cout.flush();

        // Create Echo interface
        std::cout << "Creating EchoConfig..." << std::endl;
        std::cout.flush();

        hy::EchoConfig config;

        std::cout << "Setting echo_id..." << std::endl;
        std::cout.flush();

        config.echo_id = "test_echo_1";

        std::cout << "Setting uppercase_mode..." << std::endl;
        std::cout.flush();
        config.uppercase_mode = false;

        std::cout << "Config created. Calling create_interface..." << std::endl;
        std::cout.flush();

        auto echo = manager->create_interface<hy::Echo>(config);

        std::cout << "Echo interface created!" << std::endl;
        std::cout.flush();

        // Open the interface
        if (auto res = echo.open(); !res)
        {
            std::cerr << "Failed to open echo interface: " << res.error().message << std::endl;
            return 1;
        }

        std::cout << "Echo interface opened successfully" << std::endl;

        // Set up receive callback
        std::atomic<int> received_count{0};
        std::string last_received;

        echo->set_callback([&](const std::string& frame)
        {
            std::cout << "Received echo: " << frame << std::endl;
            last_received = frame;
            received_count++;
        });

        // Test 1: Simple echo
        std::cout << "\n=== Test 1: Simple Echo ===" << std::endl;
        std::string test_msg1 = "Hello, World!";
        hy::detail::WriteCallback write_cb1 = [&test_msg1](ssize_t result)
        {
            if (result > 0)
            {
                std::cout << "Sent: " << test_msg1 << " (" << result << " bytes)" << std::endl;
            }
            else
            {
                std::cerr << "Send failed with error code: " << result << std::endl;
            }
        };

        echo->send(test_msg1, write_cb1);

        // Wait for response
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (last_received == test_msg1)
        {
            std::cout << "✓ Test 1 PASSED: Echo matches" << std::endl;
        }
        else
        {
            std::cerr << "✗ Test 1 FAILED: Expected '" << test_msg1 << "', got '" << last_received << "'" << std::endl;
            return 1;
        }

        // Test 2: Reconfigure to uppercase mode
        std::cout << "\n=== Test 2: Uppercase Echo ===" << std::endl;
        hy::EchoConfig new_config = config;
        new_config.uppercase_mode = true;

        if (auto res = echo.reconfigure(new_config); !res)
        {
            std::cerr << "Failed to reconfigure: " << res.error().message << std::endl;
            return 1;
        }

        std::string test_msg2 = "lowercase text";
        hy::detail::WriteCallback write_cb2 = [&test_msg2](ssize_t result)
        {
            if (result > 0)
            {
                std::cout << "Sent: " << test_msg2 << " (" << result << " bytes)" << std::endl;
            }
        };

        echo->send(test_msg2, write_cb2);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::string expected_upper = "LOWERCASE TEXT";
        if (last_received == expected_upper)
        {
            std::cout << "✓ Test 2 PASSED: Uppercase echo works" << std::endl;
        }
        else
        {
            std::cerr << "✗ Test 2 FAILED: Expected '" << expected_upper << "', got '" << last_received << "'" <<
                std::endl;
            return 1;
        }

        // Test 3: Close and verify
        std::cout << "\n=== Test 3: Close Interface ===" << std::endl;
        if (auto res = echo.close(); !res)
        {
            std::cerr << "Failed to close: " << res.error().message << std::endl;
            return 1;
        }
        if (!echo.is_open())
        {
            std::cout << "✓ Test 3 PASSED: Interface closed successfully" << std::endl;
        }
        else
        {
            std::cerr << "✗ Test 3 FAILED: Interface still reports as open" << std::endl;
            return 1;
        }

        std::cout << "\n=== All Tests PASSED ===" << std::endl;
        std::cout << "Total messages received: " << received_count << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
