add_library(HyComm_Detail
        Session.cpp
)

target_include_directories(HyComm_Detail PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(HyComm_Detail PUBLIC
        tl::expected
        iceoryx2-cxx::shared-lib-cxx
)