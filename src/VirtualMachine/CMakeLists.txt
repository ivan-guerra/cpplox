cmake_minimum_required(VERSION 3.13...3.22)

project(VirtualMachine DESCRIPTION "Virtual machine executing lox bytecode"
                       LANGUAGES   CXX
)

add_library(${PROJECT_NAME} STATIC VirtualMachine.cc Stack.cc)

target_include_directories(${PROJECT_NAME}
    PUBLIC
       "${LOX_INCLUDE_DIR}/VirtualMachine"
)

if(DEBUG_TRACE_EXECUTION)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            -DDEBUG_TRACE_EXECUTION
    )
endif(DEBUG_TRACE_EXECUTION)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Werror
        -Wextra
        -Wno-int-in-bool-context
        -g
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
        Compiler
)
