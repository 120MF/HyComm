add_executable(test_echo_interface
        test_echo_interface.cpp
)

target_link_libraries(test_echo_interface PUBLIC
        HyComm_IpcShared
        HyComm_Detail
        HyComm_Common
        HyComm_Protocols
        HyComm_Builders
)

# Add the test to CTest
add_test(NAME EchoInterfaceTest COMMAND test_echo_interface)

# Set timeout for the test (in seconds)
set_tests_properties(EchoInterfaceTest PROPERTIES TIMEOUT 30)

