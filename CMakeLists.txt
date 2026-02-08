# CMakeList.txt : CMake project for InfraLite-MockServer, include source and define
# project specific logic here.
#
#cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
#if (POLICY CMP0141)
#  cmake_policy(SET CMP0141 NEW)
#  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
#endif()

#project ("InfraLite-MockServer")

# Add source to this project's executable.
#add_executable (InfraLite-MockServer "src/InfraLite-MockServer.cpp"  "src/server/server.cpp" "src/server/router.cpp" "src/server/request.cpp" "src/server/response.cpp")

#if (CMAKE_VERSION VERSION_GREATER 3.12)
#  set_property(TARGET InfraLite-MockServer PROPERTY CXX_STANDARD 20)
#endif()

# TODO: Add tests and install targets if needed.

cmake_minimum_required(VERSION 3.15)
project(InfraLite-MockServer)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)

# Add include directories
include_directories(include)

# Add subdirectories for modular code
add_subdirectory(src/server)
# Later you will add:
# add_subdirectory(src/db)
# add_subdirectory(src/ui)

# Main executable (entry point)
add_executable(infralite src/InfraLite-MockServer.cpp)

# Link server module to main
target_link_libraries(infralite server)
