cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
project(LLVM360)

# msvc stuff
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(LLVM_ENABLE_PROJECTS "") # disable all subprojects
set(LLVM_TARGETS_TO_BUILD "X86;AArch64;ARM") # only build these targets
set(BUILD_SHARED_LIBS OFF) # don't need shared libs
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
add_subdirectory(external/llvm/llvm ${CMAKE_CURRENT_BINARY_DIR}/llvm_build EXCLUDE_FROM_ALL)
set(LLVM_DIR "${CMAKE_CURRENT_BINARY_DIR}/llvm_build/lib/cmake/llvm/")
set(STATIC_LINK_LLVM ON CACHE BOOL "" FORCE)
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
add_definitions(-DLLVM_STATIC)




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
