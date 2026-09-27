cmake_minimum_required(VERSION 3.18)
project(spark VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(GIO REQUIRED gio-2.0)

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9b
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG v1.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(implot)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMPLOT_DIR ${implot_SOURCE_DIR})

configure_file(
    ${CMAKE_SOURCE_DIR}/src/version.hpp.in
    ${CMAKE_BINARY_DIR}/generated/version.hpp
)

add_executable(spark
    src/main.cpp
    src/boot_imgui.cpp
    src/main_window.cpp
    src/cpu_stats.cpp
    src/ram_stats.cpp
    src/disk_stats.cpp
    src/tray_indicator.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
)

target_include_directories(spark PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMPLOT_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(spark PRIVATE
    ${GLFW_LIBRARIES}
    ${GIO_LIBRARIES}
    OpenGL::GL
    dl
    pthread
)

target_link_directories(spark PRIVATE ${GLFW_LIBRARY_DIRS} ${GIO_LIBRARY_DIRS})
target_compile_options(spark PRIVATE ${GIO_CFLAGS_OTHER})
