cmake_minimum_required(VERSION 3.15)
project(LLVM360)

# --------- CHANGE THIS TO YOUR OWN LLVM INSTALLATION PATH ---------
#set(LLVM_DIR_INST "C:/LLVM19") 
#set(LLVM_DIR ${LLVM_DIR_INST}/lib/cmake/llvm) 
#find_package(LLVM REQUIRED CONFIG)
#set(LLD_DIR ${LLVM_DIR_INST}/lib/cmake/lld)
#find_package(LLD REQUIRED CONFIG)
#message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
#message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
#include_directories(${LLVM_INCLUDE_DIRS})
#add_definitions(${LLVM_DEFINITIONS})
#add_definitions(-D_CRT_SECURE_NO_WARNINGS)
#add_definitions(-DLLVM_STATIC)


set(LLVM_ENABLE_PROJECTS "") # disable all subprojects
set(LLVM_TARGETS_TO_BUILD "X86;AArch64;ARM") # only build these targets

# don't need shared libs
set(BUILD_SHARED_LIBS OFF)


set(LLVM_INCLUDE_TOOLS OFF)
set(LLVM_INCLUDE_TESTS OFF)
set(LLVM_INCLUDE_EXAMPLES OFF)
set(LLVM_INCLUDE_DOCS OFF)
set(LLVM_ENABLE_BINDINGS OFF)
set(LLVM_BUILD_LLVM_DYLIB OFF)
set(LLVM_LINK_LLVM_DYLIB OFF)
set(LLVM_ENABLE_RTTI ON) # rtti


set(LLVM_ENABLE_ASSERTIONS OFF CACHE BOOL "" FORCE) # optional, should speed up a bit


# build llvm submodule as static lib
add_subdirectory(external/llvm/llvm)
add_definitions(-DLLVM_STATIC)
include_directories(external/llvm/llvm/include)
# ------------------------------------------------------------------


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)




# temp
set(CMAKE_SYSTEM_VERSION 10.0.19041.0)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
set(REMOVE_TARGETS
    AArch64TargetParserTableGen
    acc_gen
    ARMTargetParserTableGen
    intrinsics_gen
    omp_gen
    RISCVTargetParserTableGen
)
foreach(TARGET_NAME IN LISTS REMOVE_TARGETS)
    if(TARGET ${TARGET_NAME})
        set_property(TARGET ${TARGET_NAME} PROPERTY EXCLUDE_FROM_ALL TRUE)
        set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "llvm_gen")
    endif()
endforeach()

add_subdirectory(llvm360/Naive+)
#add_subdirectory(llvm360/Emulator)
