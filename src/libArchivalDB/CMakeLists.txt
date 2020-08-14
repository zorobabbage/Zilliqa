add_library (ArchivalDB  ArchivalDB.cpp)

target_include_directories (ArchivalDB PUBLIC ${PROJECT_SOURCE_DIR}/src ${G3LOG_INCLUDE_DIRS})
target_link_libraries(ArchivalDB PRIVATE mongo::bsoncxx_shared)
target_link_libraries(ArchivalDB PUBLIC Server mongo::mongocxx_shared)