project(qsvn)
set(PACKAGENAME "${CMAKE_PROJECT_NAME}")
cmake_minimum_required(VERSION 3.1.0)
set(CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED Widgets)

FILE(GLOB_RECURSE header_list "src/*.h")
FILE(GLOB_RECURSE cpp_list "src/*.cpp")

qt5_wrap_cpp(MOC ${cpp_list})

#set(RESOURCE_DIR res/resources.qrc)
#qt5_wrap_ui(UIC widget.ui)
#qt5_add_resources(RCC resources.qrc)

add_executable(qsvn ${cpp_list} ${header_list} src/utils/stringutils.cpp src/utils/stringutils.h)
target_link_libraries(qsvn Qt5::Widgets)

