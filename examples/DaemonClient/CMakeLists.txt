add_executable(HC_Example_DaemonClient
        main.cpp
)
target_link_libraries(HC_Example_DaemonClient
        iceoryx2-cxx::shared-lib-cxx
        HyComm_IpcShared
)