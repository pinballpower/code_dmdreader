#!/bin/bash
if [ "$1" == "clean" ]; then
  find . -iwholename '*cmake*' -not -name CMakeLists.txt -delete
  rm Makefile
fi

INCLUDE_PATH="/usr/include:/usr/include/stb:/usr/include/libdrm:/opt/vc/include"
export CMAKE_INCLUDE_PATH=$INCLUDE_PATH
cmake -DCMAKE_BUILD_TYPE=Release -DMAX_LOG_LEVEL=info .
make -j 4
