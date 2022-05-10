cmake_minimum_required(VERSION 3.13...3.22)

project(Object DESCRIPTION "Representation of a Lox objects"
               LANGUAGES   CXX
)

add_library(${PROJECT_NAME} STATIC Object.cc)

target_include_directories(${PROJECT_NAME}
    PUBLIC
       "${LOX_INCLUDE_DIR}/Object"
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Werror
        -Wextra
        "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Value
        Chunk
)
