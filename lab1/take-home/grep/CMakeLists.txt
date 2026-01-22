cmake_minimum_required(VERSION 3.22)
project(seekme_lab1_takehome_grep)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add the grep source files (boilerplate)
add_library(grep_lib
        grep/grep.c
)

# Add grep solution library (for testing)
add_library(grep_solution_lib
        grep/grep_solution.c
)

# Add grep solution test executable
add_executable(grep_test_solution
        grep/grep_test_solution.c
)

# Link grep solution library to test
target_link_libraries(grep_test_solution
        grep_solution_lib
)

# Add a custom target to run grep solution tests
add_custom_target(run_grep_solution_test
        COMMAND grep_test_solution
        DEPENDS grep_test_solution
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Add a test target to be used with CTest
enable_testing()
add_test(NAME GrepTests COMMAND grep_test_solution)

# Ensure that `make` will build and run tests
add_dependencies(run_grep_solution_test grep_test_solution)