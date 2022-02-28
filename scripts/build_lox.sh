#!/bin/bash

# This script builds the lox interpreter. build_lox.sh can process a number
# of commandline parameters. Run 'build.sh -h' to see all the options.

# Print a help message to the console.
Help()
{
    echo "Build the lox interpreter."
    echo
    echo "usage: build_lox.sh [d|h]"
    echo "options:"
    echo "d    Build project documentation (default OFF)."
    echo "h    Print this help message."
}

BUILD_DOC="OFF"

while getopts ":hd" flag
do
    case "${flag}" in
        d) BUILD_DOC="ON";;
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
    cmake ../ -DBUILD_DOC=${BUILD_DOC} && \
    make -j$(nproc) all                && \
    make install

    # Exit if any of the above commands fails.
    if [ $? -ne 0 ];
    then
        exit 1
    fi
popd
