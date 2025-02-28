add_subdirectory(backend)

add_executable(KNDB main.cpp)

target_link_libraries(KNDB backend_lib)
