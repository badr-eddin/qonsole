cmake_minimum_required(VERSION 3.28)

project(qonsole LANGUAGES CXX)

add_library(qonsole INTERFACE)

target_include_directories(qonsole
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

install(
    TARGETS qonsole
    EXPORT qonsoleTargets
)

install(
    FILES include/qonsole/qonsole.hpp
    DESTINATION include/qonsole
)

# cmake metadata (for find_package)
install(
    EXPORT qonsoleTargets
    FILE qonsoleTargets.cmake
    NAMESPACE qonsole::
    DESTINATION lib/cmake/qonsole
)
