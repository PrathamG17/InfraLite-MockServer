# Server module
add_library(server
    server.cpp
    router.cpp
    request.cpp
    response.cpp
    config_loader.cpp
    file_handler.cpp 
    logger.cpp)

target_include_directories(server PUBLIC ${CMAKE_SOURCE_DIR}/include/server)