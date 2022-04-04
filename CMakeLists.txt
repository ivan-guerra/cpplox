cmake_minimum_required(VERSION 3.13...3.22)

# Require out-of-source builds.
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

project(cpplox VERSION 1.0
              DESCRIPTION "A C++ implementation of the Lox language."
)

set(LOX_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include"
    CACHE STRING "${PROJECT_NAME} include directory.")

add_subdirectory(docs)
add_subdirectory(lox)
