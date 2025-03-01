add_subdirectory(backend)

add_executable(KNDB main.cpp
        ../test/iohandler_test.cpp)

target_link_libraries(KNDB backend)
