find_package(iceoryx2-cxx QUIET)
if (NOT iceoryx2-cxx_FOUND)
    FetchContent_Declare(iceoryx2
            GIT_REPOSITORY https://github.com/eclipse-iceoryx/iceoryx2.git
            GIT_TAG main
            GIT_SHALLOW true
            EXCLUDE_FROM_ALL
    )
    set(EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(iceoryx2)
    find_package(iceoryx2-cxx REQUIRED)
endif ()