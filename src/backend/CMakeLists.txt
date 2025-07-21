add_library(backend STATIC
        Table.cpp
        StorageEngine.cpp
        SchemaPage.cpp
        IOHandler.cpp
        FSMPage.cpp
        TablePage.cpp
        Pager.cpp
        FreeSpaceMap.cpp
        PageCache.cpp
)

target_include_directories(backend PUBLIC ${CMAKE_SOURCE_DIR}/src/backend)
target_include_directories(backend PUBLIC ${CMAKE_SOURCE_DIR}/src)
