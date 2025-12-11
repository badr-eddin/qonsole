cmake_minimum_required(VERSION 3.28)

project(qonsole)

set(CMAKE_CXX_STANDARD 20)


find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
pkg_check_modules(TSM REQUIRED libtsm)

include_directories("include" PRIVATE ${TSM_INCLUDE_DIRS})
file(GLOB_RECURSE SRC_FILES include/*.hpp)  # for automoc

set(CMAKE_AUTOMOC ON)  # Ensures MOC is processed automatically
set(CMAKE_AUTOUIC ON)  # Required if using Qt Designer files (.ui)
set(CMAKE_AUTORCC ON)  # Required if using Qt resources (.qrc)

add_executable(qonsole main.cpp ${SRC_FILES})

target_link_libraries(qonsole Qt6::Core Qt6::Gui Qt6::Widgets)
target_link_libraries(qonsole Qt6::Widgets ${TSM_LIBRARIES} util)

set(CMAKE_EXE_LINKER_FLAGS "-Os")  # optimize
