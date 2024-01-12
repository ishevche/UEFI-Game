# UEFI-games with boot-loader functionality

### Authors: 
Ivan Shevchnko - https://github.com/ishevche

Yaroslav Klym - https://github.com/KOlegaBB

Viktoria Kocherkevych - https://github.com/ViktoryaK

### Introduction

This project demonstrates and describes the implementation of games with boot-loading capabilities on UEFI. For more theoretical information, visit our [Notion documentation](https://chartreuse-liquid-3a1.notion.site/UEFI-3946645965bd4d42a41e4ee293da3184?pvs=4)

### Games

We have implemented 3 different UEFI games:

#### Wordle

A simple text game analog to a popular Wordle game. Upon the start of the game, the random 5-letter word is chosen, and the player gets 6 attempts to guess it. After each guess, the game responds
to the player, specifying letters placed in the right and wrong places and letters that are not present in the word at all.
In our implementation, all possible words are read from the configuration file words.txt located in the root directory of the file system. After entering the word, each letter is marked
with a `+` sign if it is in the right place, with `?` if it is in the wrong place, and `_` if the letter is not present in the word.

![wordle](/resources/Wordle.png)

#### Configurations

Words list:
```
resources/words.txt
```
### 2D maze

A simple GUI game where you have to solve a labyrinth to boot the operating system. There are two versions of this game in the project. First, which you can find in the directory `Maze`, launch hardcoded boot-loader 
using UEFIBootManagerLib from EDK2. The second, which you can find in the directory `BootManager2D`, is updated with textures and gives you an opportunity to boot chosen operating systems from the config file.

![maze v1](/resources/2d_game.png)

![maze v2](/resources/2d_game_updated.png)
#### Configurations

Sprite of character for Maze and BootManager2D:
```
resources/Sprite.txt
```
Maze map for Maze:
```
resources/labyrinth.txt
```
Maze map for BootManager2D:
```
resources/MazeConfig.conf
```
Boot entries for BootManager2D:
```
resources/2DConfig.txt
```
Textures for BootManager2D:
```
resources/textures
```
### 3D maze

This game is the 3D representation of the previous 2D labyrinth with the same ability to boot the operating system the user finds in the labyrinth.


![3D maze](/resources/3D_game.png)

#### Configurations

Maze map for BootManager3D:
```
resources/MazeConfig.conf
```
Boot entries for BootManager3D:
```
resources/3DConfig.txt
```
Textures for BootManager3D:
```
resources/textures
```
### Installation

1. Install [EDK2](https://github.com/tianocore/edk2)
2. Clone this repository as a subdirectory of EDKII called `Wordle`

### Compilation

In order to build the project, the EDKII Workspace should be activated first (`edksetup` script located at the root directory of EDKII).
After that, the makefile can be called with target `build` target:
```
makefile -f ${WORKSPACE}/Wordle/Application/<target-application>/Makefile build
```
To start the `QEMU`, the makefile can be called without specifying the target.

### Credits

In our project, we used textures from [Retro Texture Pack](https://little-martian.itch.io/retro-texture-pack) by [Little Martian](https://itch.io/profile/little-martian). To use the whole texture pack, you can buy it [here](https://little-martian.itch.io/retro-texture-pack).
