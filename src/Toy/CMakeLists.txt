cmake_minimum_required(VERSION 3.13...3.22)

project(Toy DESCRIPTION "A Hello World toy program."
            LANGUAGES   CXX
)

add_library(${PROJECT_NAME} STATIC Toy.cc)

target_include_directories(${PROJECT_NAME}
    PUBLIC
       "${LOX_INCLUDE_DIR}/Toy"
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Werror
        -Wextra
)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)
