cmake_minimum_required(VERSION 3.27)
project(OSProject)

add_executable(wordle Application/Wordle/Wordle.c)
add_executable(maze Application/Maze/Maze.c)

target_include_directories(wordle PRIVATE /home/ivan/src/edk2-stable202308/MdePkg/Include/)
target_include_directories(wordle PRIVATE /home/ivan/src/edk2-stable202308/MdeModulePkg/Include/)
