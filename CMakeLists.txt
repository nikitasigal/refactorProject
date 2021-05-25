cmake_minimum_required(VERSION 3.19)
project(refactorProject C)

set(CMAKE_C_STANDARD 11)

add_executable(refactorProject main.c specialSymbols.c specialSymbols.h wordHandler.c wordHandler.h newTypes.c newTypes.h definitions.h checkInitialization.c checkInitialization.h unusedTokens.c unusedTokens.h structs.c structs.h generalFunctions.c generalFunctions.h checkLooping.c checkLooping.h checkRecursion.c checkRecursion.h incorrectWriting.c incorrectWriting.h fileHandling.c fileHandling.h)