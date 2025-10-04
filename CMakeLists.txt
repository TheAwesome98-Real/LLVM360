cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
project(LLVM360)

# msvc stuff
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()



option(USE_PREBUILT_LLVM "Use prebuilt LLVM instead of building from source" ON)

add_subdirectory(external)
add_subdirectory(llvm360/Naive+)
add_subdirectory(llvm360/Emulator)
