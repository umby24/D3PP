cmake_minimum_required(VERSION 3.10)
include_directories(include/)
# set the project name
project(D3PP)

# set project standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the executable
add_executable(D3PP main.cpp src/Files.cpp include/Files.h src/Logger.cpp include/Logger.h  src/Utils.cpp include/Utils.h src/Block.cpp include/Block.h src/TaskScheduler.cpp include/TaskScheduler.h include/mingw.mutex.h src/Network.cpp include/Network.h include/mingw.thread.h include/mingw.invoke.h)