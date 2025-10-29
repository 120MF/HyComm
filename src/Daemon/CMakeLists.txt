add_executable(HyComm_Daemon
        src/main.cpp
        src/DaemonNode.cpp
        src/InterfaceManager.cpp
        src/Backends/SerialBackend.cpp
        src/Backends/EchoBackend.cpp
)

target_include_directories(HyComm_Daemon PRIVATE
        ${LIBNL3_INCLUDE_DIR}
)

target_link_libraries(HyComm_Daemon PRIVATE
        iceoryx2-cxx::shared-lib-cxx
        ${LIBNL3_LIBRARIES}
        unordered_dense::unordered_dense
        HyComm_IpcShared
        HyComm_Common
)