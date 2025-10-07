cmake_minimum_required(VERSION 3.30)
project(Emulator)


SET(SRC

    ${Runtime}
    src/Shared.h 
    src/emulator.cpp
)

add_executable(Emulator ${SRC})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Emulator PRIVATE Naive+)
