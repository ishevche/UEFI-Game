# UEFI-games with boot-loader functionality

### Authors: 
Ivan Shevchnko - https://github.com/ishevche

Yaroslav Klym - https://github.com/KOlegaBB

Viktoria Kocherkevych - https://github.com/ViktoryaK

### Introduction

This project demonstrates and describes the implementation of games with boot loading capabilities on UEFI. For more theoretical information visit our [Notion documentation](https://chartreuse-liquid-3a1.notion.site/UEFI-3946645965bd4d42a41e4ee293da3184?pvs=4)

### Games

We implemented 3 different UEFI games:

#### Wordle

A simple text game analog to a popular Wordle game. Upon the start of the game, the random 5-letter word is chosen, and the player gets 6 attempts to guess it. After each guess, the game responds
to the player, specifying letters placed in the right and wrong places and letters that are not present in the word at all.
In our implementation, all possible words are read from the configuration file words.txt located in the root directory of the file system. After entering the word, each letter is marked
with + sign if it is in the right place, with ? if it is in the wrong place and with if the letter is not present in the word.

#### Configurations

Words list:


### 2D maze

A simple GUI game, where you have to solve labirith to boot the operating system. There is two versions of this game in the project. First, which you can find in the directory Maze, launch hardcoded boot-loader 
using UEFIBootManagerLib from EDK2. Second, which you can find in the directory BootManager2D is updated with textures and give you oportunity to boot chosen operating systems from config file.

#### Configurations

Sprite of character for Maze and BootManager2D:

Maze map for Maze:

Maze map for BootManager2D:

Boot entries for BootManager2D:

Textures for BootManager2D:


### 3D maze

This game is the 3D representation of the previous 2D labyrinth with the same ability to boot the operating system the user finds in the labyrinth.

#### Configurations

Maze map for BootManager3D:

Boot entries for BootManager3D:

Textures for BootManager3D:

### Installation

1. Install [EDK2](https://github.com/tianocore/edk2)

### Compilation
