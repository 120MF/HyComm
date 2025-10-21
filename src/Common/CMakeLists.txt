add_library(HyComm_Common SHARED
        FdTransfer.cpp
)
target_include_directories(HyComm_Common PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(HyComm_Common PUBLIC tl::expected)