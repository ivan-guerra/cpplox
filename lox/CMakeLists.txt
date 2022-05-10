cmake_minimum_required(VERSION 3.13...3.22)

project(lox DESCRIPTION "Main interpreter application"
            LANGUAGES   CXX
)

add_executable(${PROJECT_NAME} Lox.cc)

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
        VirtualMachine
)

set(LOX_INSTALL_DIR "${CMAKE_SOURCE_DIR}/bin")
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${LOX_INSTALL_DIR}
)
