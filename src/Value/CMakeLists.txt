cmake_minimum_required(VERSION 3.13...3.22)

project(Value DESCRIPTION "Representation of Lox value types"
              LANGUAGES   CXX
)

add_library(${PROJECT_NAME} STATIC Value.cc)

target_include_directories(${PROJECT_NAME}
    PUBLIC
       "${LOX_INCLUDE_DIR}/Value"
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
    PRIVATE
        Object
)
