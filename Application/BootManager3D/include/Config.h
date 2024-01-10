#ifndef OSPROJECT_CONFIG_H
#define OSPROJECT_CONFIG_H

#include "Common.h"

typedef struct BOOT_ENTRY {
    CHAR16 *title;
    CHAR16 *volume_guid;
    CHAR16 *loader_name;
    CHAR16 *icon;
    CHAR16 *options;
} BOOT_ENTRY;

EFI_STATUS ParseConfigFile(IN CHAR16 *Filename, OUT BOOT_ENTRY **Result, OUT UINTN *ResultSize);

EFI_STATUS ParseTexturesConfigFile(IN CHAR16 *Filename, OUT CHAR16 ***TextureFiles, OUT UINTN *TexturesCount);

EFI_STATUS ParseMazeConfigFile(IN CHAR16 *Filename, OUT INTN **Numbers, OUT UINTN *Width, OUT UINTN *Height);

#endif //OSPROJECT_CONFIG_H
