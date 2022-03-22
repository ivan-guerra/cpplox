cmake_minimum_required(VERSION 3.13...3.22)

project(Resolver DESCRIPTION "Variable Resolution"
                 LANGUAGES   CXX
)

add_library(${PROJECT_NAME} STATIC Resolver.cc)

target_include_directories(${PROJECT_NAME}
    PUBLIC
       "${LOX_INCLUDE_DIR}/Resolver"
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
        AbstractSyntaxTree
        Interpreter
    PRIVATE
        Scanner
        ErrorLogger
)
