#!/bin/bash

# This script builds the lox interpreter. build_lox.sh can process a number
# of commandline parameters. Run 'build.sh -h' to see all the options.

# Print a help message to the console.
Help()
{
    echo "Build the lox interpreter."
    echo
    echo "usage: build_lox.sh [OPTION]..."
    echo "options:"
    echo -e "\td    Build project documentation (default OFF)."
    echo -e "\tg    Enable debug info (default OFF)."
    echo -e "\th    Print this help message."
}

BUILD_DOC="OFF"
BUILD_TYPE="RELEASE"
DEBUG_PRINT_CODE="OFF"
DEBUG_TRACE_EXECUTION="OFF"

while getopts ":hdg" flag
do
    case "${flag}" in
        d) BUILD_DOC="ON";;
        g) BUILD_TYPE="DEBUG"
           DEBUG_PRINT_CODE="ON"
           DEBUG_TRACE_EXECUTION="ON";;
        h) Help
           exit;;
       \?) echo "Error: Invalid option"
           Help
           exit;;
    esac
done

# Source the project configuration.
source config_lox.sh

# Create the build directory if it does not already exist.
if [ ! -d $LOX_BUILD_DIR ]
then
    mkdir -pv $LOX_BUILD_DIR
fi

# Build and install the kernel.
pushd $LOX_BUILD_DIR
    cmake ../                                                 \
          -DBUILD_DOC=${BUILD_DOC}                            \
          -DCMAKE_BUILD_TYPE=${BUILD_TYPE}                    \
          -DDEBUG_PRINT_CODE=${DEBUG_PRINT_CODE}              \
          -DDEBUG_TRACE_EXECUTION=${DEBUG_TRACE_EXECUTION} && \
    make -j$(nproc) all                                    && \
    make install

    # Exit if any of the above commands fails.
    if [ $? -ne 0 ];
    then
        exit 1
    fi
popd
