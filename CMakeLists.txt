cmake_minimum_required(VERSION 3.10)
include_directories(include/)
# set the project name
project(D3PP)
set(CMAKE_COMPILER_)
set (MALLOC_CHECK_ 3)
# set project standard
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lpthread")
endif ()

set(D3PP_SOURCE_FILES src/common/Files.cpp include/common/Files.h src/common/Logger.cpp include/common/Logger.h include/Utils.cpp include/Utils.h src/Block.cpp include/Block.h src/common/TaskScheduler.cpp include/common/TaskScheduler.h src/network/Network.cpp include/network/Network.h src/Rank.cpp include/Rank.h src/watchdog.cpp include/watchdog.h src/System.cpp include/System.h src/network/Chat.cpp include/network/Chat.h include/network/LinuxSockets.h include/network/WindowsSockets.h src/network/LinuxSockets.cpp src/network/WindowsSockets.cpp include/network/LinuxServerSockets.h include/network/WindowsServerSockets.h src/network/WindowsServerSockets.cpp include/world/Entity.h src/world/Entity.cpp include/network/Network_Functions.h src/network/Network_Functions.cpp include/network/Packets.h src/network/Packets.cpp include/world/Player.h src/world/Player.cpp include/common/Player_List.h src/common/Player_List.cpp src/network/PacketHandlers.cpp include/network/PacketHandlers.h include/Client.h src/Client.cpp src/network/LinuxServerSockets.cpp include/compression.h src/common/compression.cpp include/world/Map.h src/world/Map.cpp include/common/PreferenceLoader.h src/common/PreferenceLoader.cpp include/Command.h src/Command.cpp include/BuildMode.h src/BuildMode.cpp include/world/Teleporter.h src/world/Teleporter.cpp include/CPE.h src/CPE.cpp include/network/httplib.h include/plugins/Heartbeat.h src/plugins/Heartbeat.cpp
 include/plugins/LuaPlugin.h src/plugins/LuaPlugin.cpp  include/world/Physics.h src/world/Physics.cpp include/Build.h src/Build.cpp include/EventSystem.h src/EventSystem.cpp include/events/EventTimer.h include/events/EventClientAdd.h include/events/EventClientDelete.h include/events/EventClientLogin.h include/events/EventClientLogout.h include/events/EventEntityAdd.h include/events/EventEntityDelete.h include/events/EventEntityPositionSet.h include/events/EventEntityDie.h include/events/EventMapAdd.h include/events/EventMapActionDelete.h include/events/EventMapActionResize.h include/events/EventMapActionFill.h include/events/EventMapActionSave.h include/events/EventMapActionLoad.h include/events/EventMapBlockChange.h include/events/EventMapBlockChangeClient.h include/events/EventMapBlockChangePlayer.h include/events/EventChatMap.h include/events/EventChatAll.h include/events/EventChatPrivate.h include/events/EventEntityMapChange.h src/events/EventChatAll.cpp src/events/EventChatMap.cpp src/events/EventClientAdd.cpp src/events/EventClientDelete.cpp src/events/EventClientLogin.cpp src/events/EventClientLogout.cpp src/events/EventEntityAdd.cpp src/events/EventEntityDelete.cpp src/events/EventEntityDie.cpp include/CustomBlocks.h
 src/events/EventEntityMapChange.cpp src/events/EventEntityPositionSet.cpp src/events/EventMapActionDelete.cpp src/events/EventMapActionFill.cpp src/events/EventMapActionLoad.cpp src/events/EventMapActionResize.cpp src/events/EventMapActionSave.cpp src/events/EventMapAdd.cpp src/events/EventMapBlockChange.cpp src/events/EventMapBlockChangeClient.cpp src/events/EventMapBlockChangePlayer.cpp src/events/EventTimer.cpp include/common/ByteBuffer.h src/common/ByteBuffer.cpp include/network/NetworkClient.h src/network/NetworkClient.cpp include/common/MinecraftLocation.h src/common/MinecraftLocation.cpp include/events/EntityEventArgs.h src/events/EntityEventArgs.cpp include/common/Configuration.h src/common/Configuration.cpp src/ConsoleClient.cpp include/ConsoleClient.h src/CustomBlocks.cpp src/events/PlayerEventArgs.cpp include/events/PlayerEventArgs.h "include/lua/client.h" "src/lua/client.cpp" "include/lua/buildmode.h" "src/lua/buildmode.cpp" "src/lua/build.cpp" "include/lua/build.h" "include/lua/entity.h" "src/lua/entity.cpp" "src/lua/player.cpp" "include/lua/player.h" "src/lua/map.cpp" "include/lua/map.h" "src/lua/cpe.cpp" "include/lua/cpe.h" "src/lua/block.cpp" "include/lua/block.h" "include/lua/rank.h" "include/lua/teleporter.h" "include/lua/system.h" "include/lua/network.h" "src/lua/system.cpp" "src/lua/rank.cpp" "src/lua/network.cpp" "src/lua/teleporter.cpp" src/world/IMapProvider.cpp include/world/IMapProvider.h src/world/D3MapProvider.cpp include/world/D3MapProvider.h src/world/MapActions.cpp src/world/BlockChangeQueue.cpp include/world/BlockChangeQueue.h include/world/IUniqueQueue.h src/world/IUniqueQueue.cpp src/world/PhysicsQueue.cpp include/world/PhysicsQueue.h include/world/TimeQueueItem.h include/world/ChangeQueueItem.h src/network/Server.cpp include/network/Server.h include/network/IPacket.h include/network/packets/HandshakePacket.h include/network/packets/PingPacket.h include/network/packets/BlockChangePacket.h
 "src/files/D3Map.cpp" "include/files/D3Map.h" "include/common/Vectors.h" include/world/MapActions.h include/world/MapPermissions.h include/world/MapEnvironment.h
 src/network/packets/BlockChangePacket.cpp include/network/packets/ChatPacket.h  src/network/packets/ChatPacket.cpp include/network/packets/CustomBlockSupportLevelPacket.h include/network/packets/ExtEntryPacket.h include/network/packets/ExtInfoPacket.h include/network/packets/PlayerClickedPacket.h include/network/packets/PlayerTeleportPacket.h include/network/packets/TwoWayPingPacket.h include/generation/flatgrass.cpp include/common/UndoItem.h include/world/FillState.h include/plugins/LuaState.h include/plugins/PluginManager.h src/plugins/LuaState.cpp src/plugins/PluginManager.cpp src/plugins/Cplugin.cpp include/plugins/Cplugin.h)

# add the executable
add_executable(D3PP main.cpp ${D3PP_SOURCE_FILES})


#______________________________________________________________
# WINDOWS (MSVC) BUILD REQUIREMENTS
#--------------------------------------------------------------
if (MSVC)
    find_package(ZLIB REQUIRED)
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    target_link_libraries(D3PP unofficial::sqlite3::sqlite3)
    target_link_libraries(D3PP ZLIB::ZLIB)
endif ()

#______________________________________________________________
# All Target BUILD REQUIREMENTS
#--------------------------------------------------------------
find_package(Lua REQUIRED)
if (Lua_FOUND)
    include_directories(${Lua_INCLUDE_DIRS})
    target_link_libraries(D3PP ${LUA_LIBRARIES})

    MESSAGE(STATUS "Lua found at: " ${LUA_LIBRARIES})
endif ()
#______________________________________________________________
# *NIX BUILD REQUIREMENTS
#--------------------------------------------------------------
if (UNIX)
    FIND_PACKAGE ( Threads REQUIRED )
    target_link_libraries(D3PP z)
    target_link_libraries(D3PP sqlite3)
endif()
#---------------------------------------------------------------

#______________________________________________________________
# WINDOWS (Mingw) BUILD REQUIREMENTS
#--------------------------------------------------------------
if (NOT MSVC AND WIN32)
    MESSAGE(STATUS "Windows MinGW")
    find_package( Threads )
    find_package(SQLite3)
    find_package(ZLIB REQUIRED)
    target_link_libraries(D3PP ws2_32 wsock32 sqlite3 z)
    if (ZLIB_FOUND)
       include_directories(${ZLIB_INCLUDE_DIRS})
       MESSAGE(STATUS "ZLIB DIR: " ${ZLIB_LIBRARIES})
    endif()
endif ()

#______________________________________________________________
# GOOGLE TEST REQUIREMENTS
#--------------------------------------------------------------
enable_testing()

add_executable(
  hello_test
  ${D3PP_SOURCE_FILES}
  Testing/hello_test.cc
  Testing/utils_test.cc
  Testing/common/MinecraftLocationTest.cc
  Testing/files/d3map_test.cc
  Testing/world/entity_test.cc
  Testing/world/mapactions_test.cc
  Testing/world/PhysicsQueueTest.cc
 "src/files/D3Map.cpp" "include/files/D3Map.h" "include/common/Vectors.h" Testing/world/IUniqueQueueTest.cc)

target_link_libraries(
  hello_test
  gtest_main
)
if (MSVC) 
    target_link_libraries(hello_test unofficial::sqlite3::sqlite3)
    target_link_libraries(hello_test ZLIB::ZLIB)
    target_link_libraries(hello_test ws2_32 wsock32)
else ()
    target_link_libraries(hello_test z)
    target_link_libraries(hello_test sqlite3)
endif ()

include_directories(${Lua_INCLUDE_DIRS})
target_link_libraries(hello_test ${LUA_LIBRARIES})


include(GoogleTest)
gtest_discover_tests(hello_test)