cmake_minimum_required(VERSION 3.15)
project(external)



# i'll probably move each external lib to its own cmake file when it gets bigger

#
#   LLVM
#


if(USE_PREBUILT_LLVM)
    # Point to your built LLVM directory
    set(LLVM_DIR "${CMAKE_CURRENT_BINARY_DIR}/llvm_build/lib/cmake/llvm/" CACHE PATH "Path to LLVM CMake files")
    
    find_package(LLVM REQUIRED CONFIG)
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
    add_definitions(-DLLVM_STATIC)
    
    message(STATUS "Using prebuilt LLVM from: ${LLVM_DIR}")
else()
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
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/llvm/llvm ${CMAKE_CURRENT_BINARY_DIR}/llvm_build EXCLUDE_FROM_ALL)
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
endif()

# List of LLVM libs you need:
llvm_map_components_to_libnames(LLVM_LIBS
  core
  support
  target
  irreader
  asmparser
  tablegen
  x86codegen
  x86info  
  mc
  mcjit
  mcparser
  nativecodegen
  object
  passes
)

# https://github.com/RPCS3/rpcs3/blob/master/3rdparty/llvm/CMakeLists.txt#L108
add_library(ext_llvm INTERFACE)
target_link_libraries(ext_llvm INTERFACE ${LLVM_LIBS})
target_include_directories(ext_llvm INTERFACE ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(ext_llvm INTERFACE ${LLVM_DEFINITIONS_LIST} LLVM_AVAILABLE)
