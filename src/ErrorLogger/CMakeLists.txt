cmake_minimum_required(VERSION 3.13...3.22)

project(ErrorLogger DESCRIPTION "Error logging utility"
                    LANGUAGES   CXX
)

add_library(${PROJECT_NAME} STATIC ErrorLogger.cc)

target_include_directories(${PROJECT_NAME}
    PUBLIC
       "${LOX_INCLUDE_DIR}/ErrorLogger"
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
