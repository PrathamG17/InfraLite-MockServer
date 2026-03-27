# Server module
add_library(server
    server.cpp
    router.cpp
    request.cpp
    response.cpp
    config_loader.cpp
    file_handler.cpp 
    logger.cpp 
    threadpool.cpp 
    JwtHandler.cpp 
    TLSContext.cpp 
    SecurityManager.cpp)

target_include_directories(server PUBLIC ${CMAKE_SOURCE_DIR}/include/server)

find_package(OpenSSL REQUIRED)
target_link_libraries(server OpenSSL::SSL OpenSSL::Crypto)
