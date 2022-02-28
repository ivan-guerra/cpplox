cmake_minimum_required(VERSION 3.13...3.22)

# This CMakeLists.txt was taken from here
# https://vicrucann.github.io/tutorials/quick-cmake-doxygen/

# Indicate the documentation build as an option and set it to OFF by default.
option(BUILD_DOC "Build documentation" OFF)

# Check if Doxygen is installed.
find_package(Doxygen)
if (DOXYGEN_FOUND)
    if (BUILD_DOC)
        # Set input and output files.
        set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in")
        set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

        # Request to configure the file.
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started.")

        # Note the option ALL which allows to build the docs together with the
        # application.
        add_custom_target(Docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else (BUILD_DOC)
        message("BUILD_DOC=OFF, project documentation will not be built.")
    endif (BUILD_DOC)
else (DOXYGEN_FOUND)
  message("Please install Doxygen to build the documentation.")
endif (DOXYGEN_FOUND)
