cmake_minimum_required(VERSION 3.15)
project(Runtime)

# Collect source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


# imgui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../external/imgui)
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/sdl2)


if(MSVC)
    # Debug build flags
    set(CMAKE_C_FLAGS_DEBUG "/ZI /Od /Ob0 /DNDEBUG" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "/ZI /Od /Ob0 /DNDEBUG" CACHE STRING "" FORCE)
    
    # Release build flags (optimization)
    set(CMAKE_C_FLAGS_RELEASE "/O2" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2" CACHE STRING "" FORCE)
endif()



set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp   
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp 
)


# Create a shared library
add_library(Runtime SHARED ${SOURCES} ${IMGUI_SRC})


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


target_link_libraries(Runtime PRIVATE d3d12 d3d11 dxgi dxguid)

# Link dependencies (if any)
target_precompile_headers(Runtime PRIVATE src/pch.h)

# Organize files into folders/filters matching the src/ directory
foreach(SOURCE_FILE IN LISTS SOURCES)
    get_filename_component(SOURCE_PATH "${SOURCE_FILE}" PATH)
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" "${SOURCE_PATH}")
    string(REPLACE "/" "\\" FILTER_GROUP "${REL_PATH}")
    source_group("${FILTER_GROUP}" FILES "${SOURCE_FILE}")
endforeach()
