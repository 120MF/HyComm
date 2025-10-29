add_library(HyComm_Builders
        SerialBuilder.cpp
)

target_include_directories(HyComm_Builders PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(HyComm_Builders PUBLIC
        HyComm_IpcShared
        HyComm_Protocols
        HyComm_Detail
)

