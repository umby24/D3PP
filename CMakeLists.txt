cmake_minimum_required(VERSION 3.10)
include_directories(include/)
# set the project name
project(D3PP)

# set project standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lpthread")
#______________________________________________________________
# *NIX BUILD REQUIREMENTS
#--------------------------------------------------------------
if (UNIX)
    FIND_PACKAGE ( Threads REQUIRED )
endif()
#---------------------------------------------------------------

#______________________________________________________________
# WINDOWS BUILD REQUIREMENTS
#--------------------------------------------------------------
if (WIN32)
    link_libraries(ws2_32 wsock32)
    find_package(ZLIB REQUIRED)
    find_package( Threads )
    if (ZLIB_FOUND)
        include_directories(${ZLIB_INCLUDE_DIRS})
        MESSAGE(STATUS "ZLIB DIR: " ${ZLIB_LIBRARIES})
    endif()
endif ()
# add the executable
add_executable(D3PP main.cpp src/Files.cpp include/Files.h src/Logger.cpp include/Logger.h include/Utils.cpp include/Utils.h src/Block.cpp include/Block.h src/TaskScheduler.cpp include/TaskScheduler.h src/Network.cpp include/Network.h src/Rank.cpp include/Rank.h src/Mem.cpp include/Mem.h src/watchdog.cpp include/watchdog.h src/System.cpp include/System.h src/Chat.cpp include/Chat.h include/network/LinuxSockets.h include/network/WindowsSockets.h src/network/LinuxSockets.cpp src/network/WindowsSockets.cpp include/network/LinuxServerSockets.h include/network/WindowsServerSockets.h src/network/WindowsServerSockets.cpp src/NetworkClientInput.cpp include/Entity.h src/Entity.cpp include/Network_Functions.h src/Network_Functions.cpp include/Packets.h src/Packets.cpp src/NetworkClientOutput.cpp include/Player.h src/Player.cpp include/Player_List.h src/Player_List.cpp src/PacketHandlers.cpp include/PacketHandlers.h include/Client.h src/Client.cpp src/network/LinuxServerSockets.cpp)

target_link_libraries(D3PP sqlite3)