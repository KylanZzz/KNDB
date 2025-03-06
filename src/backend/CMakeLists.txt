add_library(backend STATIC
        Table.cpp
        Table.hpp
        Pager.hpp
        Btree.cpp
        Btree.hpp
        Schema.cpp
        Schema.hpp
        Page.hpp
        SchemaPage.hpp
        SchemaPage.cpp
        IOHandler.cpp
        IOHandler.hpp
        FSMPage.cpp
        FSMPage.hpp
        kndb_types.hpp
        constants.hpp
        TablePage.cpp
        TablePage.hpp
        BtreeNodePage.cpp
        BtreeNodePage.hpp
)

target_include_directories(backend PUBLIC ${CMAKE_SOURCE_DIR}/src/backend)
