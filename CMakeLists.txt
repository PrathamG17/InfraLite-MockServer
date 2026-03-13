cmake_minimum_required(VERSION 3.15)
project(InfraLite-MockServer)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)

# Add include directories
include_directories(include)

# Ensure MSVC compiles with UTF-8
if (MSVC)
    add_compile_options(/utf-8)
endif()

# Add subdirectories for modular code
add_subdirectory(src/server)
# Later you will add:
add_subdirectory(src/db)
# add_subdirectory(src/ui)

# Main executable (entry point)
add_executable(infralite src/InfraLite-MockServer.cpp)

# Link server module to main
target_link_libraries(infralite server db)
