#!/bin/bash

set -o nounset
set -o errexit


TARGET=MandelbrotGame


# Change to the directory of the script
cd "$( dirname "${BASH_SOURCE[0]}" )" 

rm -rf build/*

mkdir --parents build
cd build
cmake ..

cmake --build . --target "${TARGET}"


./Debug/${TARGET}.exe