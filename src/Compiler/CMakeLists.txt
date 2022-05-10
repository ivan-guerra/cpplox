cmake_minimum_required(VERSION 3.13...3.22)

project(Compiler DESCRIPTION "Lox bytecode compiler"
                 LANGUAGES   CXX
)

add_library(${PROJECT_NAME} STATIC Compiler.cc)

if(DEBUG_PRINT_CODE)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            -DDEBUG_PRINT_CODE
    )
endif(DEBUG_PRINT_CODE)

target_include_directories(${PROJECT_NAME}
    PUBLIC
       "${LOX_INCLUDE_DIR}/Compiler"
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
        Chunk
        Value
        Object
        Scanner
)
