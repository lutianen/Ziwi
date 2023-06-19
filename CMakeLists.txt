cmake_minimum_required(VERSION 3.5)
project(ziwi VERSION 0.1 LANGUAGES CXX)

# CMake 的一个自动化工具，用于处理 Qt 的用户界面文件（.ui 文件）。
# 它的作用是根据 .ui 文件自动生成对应的 C++ 代码，并将其添加到项目中。
# 这样，开发人员就可以使用自动生成的 C++ 代码来访问和操作 Qt 用户界面中的控件，而不必手动编写这些代码
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QT NAMES 指定了要查找的 Qt 版本
# REQUIRED 指定了 Qt 是必须的，如果没有找到 Qt，CMake 将会报错并停止构建
# COMPONENTS 指定了需要使用的 Qt 模块
find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)
message(STATUS Qt${QT_VERSION_MAJOR})

# add_subdirectory(3rdparty/QtAwesome)
add_subdirectory(imgCore)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS src/*.cc include/*.h)
add_executable(ziwi ${PROJECT_SOURCES})
target_link_libraries(ziwi 
    PRIVATE Qt${QT_VERSION_MAJOR}::Core 
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    
    PRIVATE LuxImageCore)
target_include_directories(ziwi PUBLIC include)


# MACOSX_BUNDLE_GUI_IDENTIFIER 指定了在 macOS 平台上生成的应用程序的唯一标识符。它的格式通常为反向 DNS 格式，例如 com.example.myapp。
# MACOSX_BUNDLE_BUNDLE_VERSION 指定了在 macOS 平台上生成的应用程序的版本号。它通常与项目的版本号相同，可以使用 CMake 变量 ${PROJECT_VERSION} 来设置。
# MACOSX_BUNDLE_SHORT_VERSION_STRING 指定了在 macOS 平台上生成的应用程序的短版本号。短版本号通常只包含主版本号和次版本号，例如 1.2。
# MACOSX_BUNDLE 和 WIN32_EXECUTABLE 分别指定了在 macOS 平台和 Windows 平台上生成的应用程序是否应该作为应用程序包（bundle）或可执行文件（executable）来处理。如果设置为 TRUE，则表示生成应用程序包；如果设置为 FALSE，则表示生成可执行文件。
# set_target_properties(ziwi PROPERTIES
#     MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
#     MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#     MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#     MACOSX_BUNDLE TRUE
#     WIN32_EXECUTABLE TRUE
# )

# install(TARGETS ziwi
#     BUNDLE DESTINATION .
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})



