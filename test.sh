#!/bin/bash


echo 3 > /proc/sys/vm/drop_caches
cmake -DCMAKE_BUILD_TYPE=Debug -B build && \
cmake --build build -j32 && \
./build/Main

cmake -DCMAKE_BUILD_TYPE=Debug -B build && \
cmake --build build -j32 && \
time ./build/Main > /dev/null


