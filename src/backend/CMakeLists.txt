add_library(backend STATIC
        Table.cpp
        StorageEngine.cpp
        SchemaPage.cpp
        IOHandler.cpp
        FSMPage.cpp
        TablePage.cpp
        Pager.cpp
)

target_include_directories(backend PUBLIC ${CMAKE_SOURCE_DIR}/src/backend)
