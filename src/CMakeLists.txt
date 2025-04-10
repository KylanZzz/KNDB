add_subdirectory(backend)

add_executable(KNDB main.cpp
        assume.hpp)

target_link_libraries(KNDB backend)
