add_executable(HC_Example_DaemonConnect
        main.cpp
)
target_link_libraries(HC_Example_DaemonConnect iceoryx2-cxx::shared-lib-cxx)