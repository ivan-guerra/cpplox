cmake_minimum_required(VERSION 3.13...3.22)

project(Parser DESCRIPTION "Lox lang parser"
               LANGUAGES   CXX
)

add_library(${PROJECT_NAME} STATIC Parser.cc)

target_include_directories(${PROJECT_NAME}
    PUBLIC
       "${LOX_INCLUDE_DIR}/Parser"
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Werror
        -Wextra
        -g
)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Scanner
        AbstractSyntaxTree
        ErrorLogger
)
