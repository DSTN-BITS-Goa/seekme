cmake_minimum_required(VERSION 3.20)

# Project information
project(seekme-lab2 VERSION 1.0 LANGUAGES C)

# Find FUSE3 library
find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE3 fuse3 REQUIRED)

# Include FUSE3 headers
include_directories(${FUSE3_INCLUDE_DIRS})

# Add the executable
add_executable(myfs myfs.c)
# add_executable(myfs myfs_solution.c)

# Link FUSE3 library
target_link_libraries(myfs ${FUSE3_LIBRARIES})

# Create test directories (tc1-tc19)
set(ALL_TEST_DIRS "")
foreach(i RANGE 1 19)
    list(APPEND ALL_TEST_DIRS ${CMAKE_BINARY_DIR}/mount_tc${i} ${CMAKE_BINARY_DIR}/root_tc${i})
endforeach()

add_custom_command(
    OUTPUT ${ALL_TEST_DIRS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ALL_TEST_DIRS}
    COMMENT "Creating test directories"
)

add_custom_target(create_test_dirs ALL DEPENDS ${ALL_TEST_DIRS})

# Unmount and remove commands for all tc dirs
set(UNMOUNT_AND_REMOVE "")
foreach(i RANGE 1 19)
    list(APPEND UNMOUNT_AND_REMOVE
        COMMAND fusermount -u ${CMAKE_BINARY_DIR}/mount_tc${i} || true
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/mount_tc${i} || true
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/root_tc${i} || true
    )
endforeach()

add_custom_target(run_tests
    DEPENDS myfs create_test_dirs
    COMMAND ${CMAKE_COMMAND} -E echo "Running tests..."
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/logs
    COMMAND python3 ${CMAKE_SOURCE_DIR}/test.py || true
    COMMAND ${CMAKE_COMMAND} -E echo "Unmounting and removing directories..."
    ${UNMOUNT_AND_REMOVE}
)

add_dependencies(run_tests myfs)
