include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
FetchContent_MakeAvailable(googletest)

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(backend_test iohandler_test.cpp test_main.cpp)

# Link against backend library
target_link_libraries(backend_test gtest_main backend)
add_test(NAME my_test COMMAND backend_test)