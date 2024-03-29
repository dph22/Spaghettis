#! /usr/bin/env bash

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# export CPUFLAGS="-march=native"; ./build.sh

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

g++ -std=c++11 main.cpp -I../libraries/belle/Source -O3 -ffast-math ${CPUFLAGS} -ldl -lpthread -lm -o tests

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
