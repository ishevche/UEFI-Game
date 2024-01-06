cmake_minimum_required(VERSION 3.27)
project(OSProject)

add_executable(wordle Application/Wordle/Wordle.c)
add_executable(maze Application/Maze/Maze.c)
add_executable(tests Application/Tests/Tests.c)

target_include_directories(wordle PRIVATE /home/ivan/src/edk2-stable202308/MdePkg/Include/
        /home/ivan/src/edk2-stable202308/MdeModulePkg/Include/)
target_include_directories(maze PRIVATE /home/ivan/src/edk2-stable202308/MdePkg/Include/
        /home/ivan/src/edk2-stable202308/MdeModulePkg/Include/)
target_include_directories(tests PRIVATE /home/ivan/src/edk2-stable202308/MdePkg/Include/
        /home/ivan/src/edk2-stable202308/MdeModulePkg/Include/)
