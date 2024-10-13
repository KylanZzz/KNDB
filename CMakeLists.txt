cmake_minimum_required(VERSION 3.26)
project(KNDB)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(KNDB
        Table.cpp
        Table.hpp
        main.cpp
        Pager.cpp
        Pager.hpp
        Btree.cpp
        Btree.hpp
        HeapFileManager.cpp
        HeapFileManager.hpp
        constants.hpp
        Schema.cpp
        Schema.hpp)
