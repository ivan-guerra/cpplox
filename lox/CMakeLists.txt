cmake_minimum_required(VERSION 3.13...3.22)

project(lox DESCRIPTION "Lox lang interpreter."
            LANGUAGES   CXX
)

add_executable(${PROJECT_NAME} Lox.cc)

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

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Toy
)

set(LOX_INSTALL_DIR "${CMAKE_SOURCE_DIR}/bin")
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${LOX_INSTALL_DIR}
)
