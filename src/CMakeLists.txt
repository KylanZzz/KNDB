add_subdirectory(backend)

add_executable(KNDB main.cpp
        assume.h)

target_link_libraries(KNDB backend)
