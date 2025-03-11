cmake_minimum_required(VERSION 3.15)
project(Runtime)

# Collect source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
    # Debug build flags
    set(CMAKE_C_FLAGS_DEBUG "/ZI /Od /Ob0 /DNDEBUG" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "/ZI /Od /Ob0 /DNDEBUG" CACHE STRING "" FORCE)
    
    # Release build flags (optimization)
    set(CMAKE_C_FLAGS_RELEASE "/O2" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2" CACHE STRING "" FORCE)
endif()

# Create a shared library
add_library(Runtime SHARED ${SOURCES})

# Set output directories to the same folder as the master project (bin/)
set_target_properties(Runtime PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Ensure proper export symbols in the DLL (for MSVC)
if(MSVC)
    target_compile_definitions(Runtime PRIVATE "RUNTIME_EXPORTS")
endif()

# Link dependencies (if any)
target_precompile_headers(Runtime PRIVATE src/pch.h)
