# Master CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

project(LLVM360)

# Prevent in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-tree builds are not supported. Run CMake from a separate directory: cmake -B build")
endif()

# Enable solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find LLVM
find_package(LLVM CONFIG REQUIRED)
if(LLVM_PACKAGE_VERSION VERSION_LESS "19.0" OR LLVM_PACKAGE_VERSION VERSION_GREATER_EQUAL "21.0")
    message(FATAL_ERROR "LLVM version ${LLVM_PACKAGE_VERSION} is incompatible. Expected version 19.x or 20.x.")
else()
    message(STATUS "Using LLVM version ${LLVM_PACKAGE_VERSION}")
endif()

# Move LLVM-generated auxiliary projects to the LLVMGen filter
get_property(LLVM_TARGETS GLOBAL PROPERTY LLVM_ALL_PROJECTS)
foreach(LLVM_PROJECT IN LISTS LLVM_TARGETS)
    if(LLVM_PROJECT MATCHES "_gen$") # Matches *_gen projects
        set_property(TARGET ${LLVM_PROJECT} PROPERTY FOLDER "LLVMGen")
    endif()
endforeach()

# Suppress unwanted targets by overriding them as empty or excluding them
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
        # Override the target as a dummy empty target
        set_property(TARGET ${TARGET_NAME} PROPERTY EXCLUDE_FROM_ALL TRUE)
        set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "llvm_gen")
    endif()
endforeach()

# Set the main project as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT LLVM360)

# Include LLVM directories
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# Add subprojects
add_subdirectory(llvm360/LLVMApp)
add_subdirectory(llvm360/Runtime)
