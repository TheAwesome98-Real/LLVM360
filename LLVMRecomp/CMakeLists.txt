# Needed for CMAKE_MSVC_RUNTIME_LIBRARY
cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-tree builds are not supported. Run CMake from a separate directory: cmake -B build")
endif()

# Hunter package configuration
option(HUNTER_ENABLED "Enable the hunter package manager to automatically download and compile LLVM" OFF)
if(HUNTER_ENABLED)
    include(cmake/HunterPackages.cmake)
endif()

project(LLVM360Recomp)

# Enable solution folder support
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "C:/LLVM19/lib/cmake/llvm" ${CMAKE_PREFIX_PATH})


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find LLVM 19
# Find LLVM without strict version checking
find_package(LLVM CONFIG REQUIRED)


if(LLVM_PACKAGE_VERSION VERSION_LESS "19.0" OR LLVM_PACKAGE_VERSION VERSION_GREATER_EQUAL "21.0")
    message(FATAL_ERROR "LLVM version ${LLVM_PACKAGE_VERSION} is incompatible. Expected version 19.x or 20.x.")
else()
    message(STATUS "Using LLVM version ${LLVM_PACKAGE_VERSION}")
endif()


# Ensure the LLVM version is at least 19.0
if(LLVM_PACKAGE_VERSION VERSION_LESS "19.0" OR LLVM_PACKAGE_VERSION VERSION_GREATER_EQUAL "20.0")
    message(FATAL_ERROR "LLVM version ${LLVM_PACKAGE_VERSION} is incompatible. Expected version 19.x.")
endif()

# Include LLVM directories
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# MSVC-specific options
if(MSVC)
    # Adjust compiler flags for better compatibility
    set(CMAKE_C_FLAGS_DEBUG "/ZI /Od /Ob0 /DNDEBUG" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "/ZI /Od /Ob0 /DNDEBUG" CACHE STRING "" FORCE)

    if(LLVM_USE_CRT_RELEASE STREQUAL "MD")
        set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
    elseif(LLVM_USE_CRT_RELEASE STREQUAL "MT")
        set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
    else()
        message(FATAL_ERROR "Unsupported LLVM_USE_CRT_RELEASE=${LLVM_USE_CRT_RELEASE}")
    endif()
endif()

# Add the executable target
add_executable(example
    src/example.cpp
)

# Link with LLVM libraries
llvm_map_components_to_libnames(LLVM_LIBS support core irreader bitwriter)
target_link_libraries(example PRIVATE ${LLVM_LIBS})

# Ensure LLVM headers and definitions are available
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Set the main project as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT example)
