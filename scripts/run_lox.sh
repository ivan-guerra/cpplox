#!/bin/bash

# This script runs the lox interpreter.

LGREEN='\033[1;32m'
LRED='\033[1;31m'
NC='\033[0m'

# Source the project configuration.
source config_lox.sh

${LOX_BIN_DIR}/lox
if [ $? -ne 0 ]; then
    echo -e "${LRED}lox did not exit cleanly. See above for details.${NC}"
    exit $?
fi
