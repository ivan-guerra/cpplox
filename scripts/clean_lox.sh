#!/bin/bash

# This script cleans up the source tree leaving it as if a fresh clone of
# the repository was made.

LGREEN='\033[1;32m'
NC='\033[0m'

# Source the project configuration.
source config_lox.sh

# Remove the binary directory.
if [ -d $LOX_BIN_DIR ]
then
    echo -e "${LGREEN}Removing '$LOX_BIN_DIR'${NC}"
    rm -r $LOX_BIN_DIR
fi

# Remove project docs.
if [ -d $LOX_DOCS_DIR ]
then
    echo -e "${LGREEN}Removing '$LOX_DOCS_DIR'${NC}"
    rm -r $LOX_DOCS_DIR
fi

# Remove the CMake build directory.
if [ -d $LOX_BUILD_DIR ]
then
    echo -e "${LGREEN}Removing '$LOX_BUILD_DIR'${NC}"
    rm -r $LOX_BUILD_DIR
fi
