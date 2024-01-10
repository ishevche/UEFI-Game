cmake_minimum_required(VERSION 3.27)
project(OSProject C)

add_executable(wordle Application/Wordle/Wordle.c)
add_executable(maze Application/Maze/Maze.c)
add_executable(xv6 Application/XV6/xv6.c)
add_executable(bmgr2D
        Application/BootManager2D/src/Main.c
        Application/BootManager2D/src/Common.c
        Application/BootManager2D/src/BMPReader.c
        Application/BootManager2D/src/DemoGame.c
        Application/BootManager2D/src/Config.c
        Application/BootManager2D/src/FileSystems.c
        Application/BootManager2D/src/Graphics.c
)
add_executable(bmgr3D
        Application/BootManager3D/src/Main.c
        Application/BootManager3D/src/Common.c
        Application/BootManager3D/src/BMPReader.c
        Application/BootManager3D/src/DemoGame.c
        Application/BootManager3D/src/Config.c
        Application/BootManager3D/src/FileSystems.c
        Application/BootManager3D/src/Graphics.c
)

target_include_directories(wordle PRIVATE /home/ivan/src/edk2-stable202308/MdePkg/Include/
        /home/ivan/src/edk2-stable202308/MdeModulePkg/Include/)
target_include_directories(maze PRIVATE /home/ivan/src/edk2-stable202308/MdePkg/Include/
        /home/ivan/src/edk2-stable202308/MdeModulePkg/Include/)
target_include_directories(xv6 PRIVATE /home/ivan/src/edk2-stable202308/MdePkg/Include/
        /home/ivan/src/edk2-stable202308/MdeModulePkg/Include/)
target_include_directories(bmgr2D PRIVATE /home/ivan/src/edk2-stable202308/MdePkg/Include/
        /home/ivan/src/edk2-stable202308/MdeModulePkg/Include/
        /home/ivan/src/edk2-stable202308/StdLib/Include/
        Application/BootManager2D/include)
target_include_directories(bmgr3D PRIVATE /home/ivan/src/edk2-stable202308/MdePkg/Include/
        /home/ivan/src/edk2-stable202308/MdeModulePkg/Include/
        /home/ivan/src/edk2-stable202308/StdLib/Include/
        Application/BootManager3D/include)
