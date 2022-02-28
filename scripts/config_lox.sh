#!/bin/bash

# This script configures the default search paths for many of the binaries
# and configuration files used by the project. Other scripts may source this
# file to find the resources that they need.

CWD=$(pwd)

# Root directory.
LOX_PROJECT_PATH=$(dirname ${CWD})

# Scripts directory.
LOX_SCRIPTS_PATH="${LOX_PROJECT_PATH}/scripts"

# Binary directory.
LOX_BIN_DIR="${LOX_PROJECT_PATH}/bin"

# Doxygen output directory.
LOX_DOCS_DIR="${LOX_PROJECT_PATH}/docs/cpplox"

# CMake build files (see build_lox.sh for more info).
LOX_BUILD_DIR="${LOX_PROJECT_PATH}/build"
