add_library(HyComm_Protocols
        SerialTraits.cpp
)

target_include_directories(HyComm_Protocols PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(HyComm_Protocols PUBLIC
        HyComm_IpcShared
)

