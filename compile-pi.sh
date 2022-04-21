#!/bin/bash
if [ "$1" == "clean" ]; then
  find . -iwholename '*cmake*' -not -name CMakeLists.txt -delete
  rm Makefile
fi

INCLUDE_PATH="/usr/include:/usr/include/stb:/usr/include/libdrm"
export CMAKE_INCLUDE_PATH=$INCLUDE_PATH
#export CPATH=$INCLUDE_PATH
cmake .
make -j 4
