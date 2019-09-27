#!/bin/sh

clang-format -i main/src/*.c main/include/*.h sim/hardware/*.c components/hardware/src/*.c components/hardware/include/*.h components/acodecs/src/acodecs.c components/acodecs/include/acodecs.h
