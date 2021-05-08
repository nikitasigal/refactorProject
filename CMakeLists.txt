cmake_minimum_required(VERSION 3.19)
project(refactorProject C)

set(CMAKE_C_STANDARD 11)

add_executable(refactorProject main.c specialSymbols.c specialSymbols.h wordHandler.c wordHandler.h)