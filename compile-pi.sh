#!/bin/bash

total_memory=$(free -b | awk 'NR==2 {print $2}')
if [ $total_memory -lt 1000000000 ]; then
 echo "Can't compile on a system with less than 1GB physical memory"
fi

if [ "$1" == "clean" ]; then
  find . -iwholename '*cmake*' -not -name CMakeLists.txt -delete
  rm Makefile
fi

INCLUDE_PATH="/usr/include:/usr/include/stb:/usr/include/libdrm:/opt/vc/include"
export CMAKE_INCLUDE_PATH=$INCLUDE_PATH
cmake -DCMAKE_BUILD_TYPE=Release -DMAX_LOG_LEVEL=info -DFORCE_SPI=1 -DFORCE_SERUM=1 -DFORCE_VNI=1 -DFORCE_XBR=1 -DFORCE_LEDMATRIX=1.
make -j 1
