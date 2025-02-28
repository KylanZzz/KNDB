add_library(backend_lib STATIC
        Table.cpp
        Table.hpp
        Pager.cpp
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
        FreeSpaceMap.cpp
        FreeSpaceMap.hpp
        FSMPage.cpp
        FSMPage.hpp
)

target_include_directories(backend_lib PUBLIC ${CMAKE_SOURCE_DIR}/src/backend)
