cmake_minimum_required(VERSION 3.26)
project(KNDB)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(KNDB
        backend/Table.cpp
        backend/Table.hpp
        backend/main.cpp
        backend/Pager.cpp
        backend/Pager.hpp
        backend/Btree.cpp
        backend/Btree.hpp
        backend/HeapFileManager.cpp
        backend/HeapFileManager.hpp
        constants.hpp
        backend/Schema.cpp
        backend/Schema.hpp)
