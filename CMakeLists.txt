cmake_minimum_required(VERSION 3.15)
project(D3PP VERSION 0.1.20 LANGUAGES CXX)

# ──────────────────────────────────────────────
# Build settings
# ──────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

if(NOT MSVC)
    add_compile_options(-pthread)
    add_link_options(-pthread)
endif()

# ──────────────────────────────────────────────
# Options
# ──────────────────────────────────────────────
option(BUILD_GUI "Build the ImGui/Vulkan/SDL2 graphical console" OFF)

# ──────────────────────────────────────────────
# Find system packages
# ──────────────────────────────────────────────
find_package(Lua REQUIRED)
message(STATUS "Lua found at: ${LUA_LIBRARIES}")

# ──────────────────────────────────────────────
# Third-party: digestpp (header-only, has CMakeLists)
# ──────────────────────────────────────────────
add_subdirectory(lib/digestpp)

if(BUILD_GUI)
    find_package(SDL2 REQUIRED)
    message(STATUS "SDL2 found at: ${SDL2_LIBRARIES}")

    find_package(Vulkan REQUIRED)
    message(STATUS "Vulkan found at: ${Vulkan_LIBRARIES}")

    # ──────────────────────────────────────────────
    # Third-party: ImGui (no upstream CMakeLists — build as static library)
    # ──────────────────────────────────────────────
    add_library(imgui STATIC
        lib/imgui/imgui.cpp
        lib/imgui/imgui_draw.cpp
        lib/imgui/imgui_tables.cpp
        lib/imgui/imgui_widgets.cpp
        lib/imgui/backends/imgui_impl_sdl2.cpp
        lib/imgui/backends/imgui_impl_vulkan.cpp
        lib/imgui/misc/cpp/imgui_stdlib.cpp
    )
    target_include_directories(imgui PUBLIC
        lib/imgui
        lib/imgui/backends
        lib/imgui/misc/cpp
        ${SDL2_INCLUDE_DIRS}
        ${Vulkan_INCLUDE_DIRS}
    )
    target_link_libraries(imgui PUBLIC
        ${SDL2_LIBRARIES}
        Vulkan::Vulkan
    )
endif()

# ──────────────────────────────────────────────
# GoogleTest (fetched at configure time)
# ──────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ──────────────────────────────────────────────
# Source files
# ──────────────────────────────────────────────
file(GLOB_RECURSE D3PP_SOURCE_FILES src/*.cpp include/*.cpp include/*.h)

if(NOT BUILD_GUI)
    list(FILTER D3PP_SOURCE_FILES EXCLUDE REGEX ".*/src/gui/.*\\.cpp$")
    list(FILTER D3PP_SOURCE_FILES EXCLUDE REGEX ".*/include/gui/.*\\.cpp$")
endif()

# Test sources exclude main.cpp and mainGui.cpp (which start the full server/GUI)
set(D3PP_TEST_SOURCE_FILES ${D3PP_SOURCE_FILES})

list(APPEND D3PP_SOURCE_FILES main.cpp)

# ──────────────────────────────────────────────
# Main executable
# ──────────────────────────────────────────────
if(WIN32)
    enable_language(RC)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")
endif()

add_executable(D3PP ${D3PP_SOURCE_FILES} ${APP_ICON_RESOURCE_WINDOWS})
set_target_properties(D3PP PROPERTIES VERSION ${PROJECT_VERSION})

target_include_directories(D3PP PRIVATE
    include/
    ${LUA_INCLUDE_DIR}
)
target_link_libraries(D3PP PRIVATE
    digestpp::digestpp
    ${LUA_LIBRARIES}
)
if(BUILD_GUI)
    target_compile_definitions(D3PP PRIVATE BUILD_GUI)
    target_link_libraries(D3PP PRIVATE imgui)
endif()

# Platform-specific link requirements
if(MSVC)
    message(STATUS "MSVC Build --")
    find_package(ZLIB REQUIRED)
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    target_link_libraries(D3PP PRIVATE unofficial::sqlite3::sqlite3 ZLIB::ZLIB)
elseif(WIN32)
    message(STATUS "Windows MinGW Build --")
    find_package(ZLIB REQUIRED)
    if(ZLIB_FOUND)
        message(STATUS "ZLIB DIR: ${ZLIB_LIBRARIES}")
    endif()
    target_link_libraries(D3PP PRIVATE ws2_32 wsock32 sqlite3 z)
else() # Unix/Linux
    target_link_libraries(D3PP PRIVATE z sqlite3)
endif()

# ──────────────────────────────────────────────
# Tests
# ──────────────────────────────────────────────
enable_testing()

add_executable(hello_test
    ${D3PP_TEST_SOURCE_FILES}
    Testing/hello_test.cc
    Testing/utils_test.cc
    Testing/Client_test.cc
    Testing/common/MinecraftLocationTest.cc
    Testing/common/ByteBufferTest.cc
    Testing/files/d3map_test.cc
    Testing/files/ClassicWorldTest.cc
    Testing/world/entity_test.cc
    Testing/world/mapactions_test.cc
    Testing/world/PhysicsQueueTest.cc
    Testing/nbt/nbt_test.cc
    Testing/world/IUniqueQueueTest.cc
    Testing/network/chat_tests.cc
)
target_include_directories(hello_test PRIVATE
    include/
    ${LUA_INCLUDE_DIR}
)
target_link_libraries(hello_test PRIVATE
    gtest_main gmock gmock_main
    digestpp::digestpp
    ${LUA_LIBRARIES}
)
if(BUILD_GUI)
    target_compile_definitions(hello_test PRIVATE BUILD_GUI)
    target_link_libraries(hello_test PRIVATE imgui)
endif()

if(MSVC)
    target_link_libraries(hello_test PRIVATE unofficial::sqlite3::sqlite3 ZLIB::ZLIB ws2_32 wsock32)
elseif(WIN32)
    target_link_libraries(hello_test PRIVATE ws2_32 wsock32 sqlite3 z)
else()
    target_link_libraries(hello_test PRIVATE z sqlite3)
endif()

include(GoogleTest)
gtest_discover_tests(hello_test)
