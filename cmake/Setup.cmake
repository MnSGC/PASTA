# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(NOT CMAKE_C_STANDARD)
        set(CMAKE_C_STANDARD 99)
    endif()

    if(NOT CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 17)
    endif()

    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wall -Wextra -Wpedantic)
    endif()

    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDE's
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

endif()

# Set up third party
include(cmake/thirdparty/SetupThirdParty.cmake)
