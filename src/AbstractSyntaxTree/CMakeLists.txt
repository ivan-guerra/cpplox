cmake_minimum_required(VERSION 3.13...3.22)

project(AbstractSyntaxTree DESCRIPTION "Lox lang abstract syntax tree"
                           LANGUAGES   CXX
)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE
       "${LOX_INCLUDE_DIR}/AbstractSyntaxTree"
)

target_compile_options(${PROJECT_NAME}
    INTERFACE
        -Wall
        -Werror
        -Wextra
        -g
)

target_compile_features(${PROJECT_NAME}
    INTERFACE
        cxx_std_17
)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
        Scanner
)
