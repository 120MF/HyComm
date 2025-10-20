add_executable(HyComm_Daemon src/main.cpp)

target_link_libraries(HyComm_Daemon iceoryx2-cxx::shared-lib-cxx)