#ifndef OSPROJECT_GRAPHICS_H
#define OSPROJECT_GRAPHICS_H
#include "Common.h"

EFI_GRAPHICS_OUTPUT_BLT_PIXEL *CutInHalf(IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Image, IN UINTN Width, IN UINTN Height);

EFI_GRAPHICS_OUTPUT_BLT_PIXEL *CombineWithTransparency(IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Foreground,
                                                       IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Background,
                                                       IN UINTN Width, IN UINTN Height);

#endif //OSPROJECT_GRAPHICS_H