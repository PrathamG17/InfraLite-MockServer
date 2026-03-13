add_library(db 
    database_handler.cpp
    route_repository.cpp
    access_log_repository.cpp
    sqlite3.c
)

set_source_files_properties(sqlite3.c PROPERTIES LANGUAGE C)

target_include_directories(db PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
