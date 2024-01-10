#ifndef OSPROJECT_GAME_H
#define OSPROJECT_GAME_H

#include "Common.h"
#include "Config.h"

#define ESCAPE_ENTRY MAX_UINTN

UINTN Game(INTN *Map, UINTN MapWidth, UINTN MapHeight,
           EFI_GRAPHICS_OUTPUT_BLT_PIXEL **Textures, UINTN TexturesCount,
           BOOT_ENTRY *BootEntries, UINTN BootEntriesCount,
           EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Sprite);


#endif //OSPROJECT_GAME_H

