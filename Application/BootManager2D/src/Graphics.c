#include "Graphics.h"


EFI_GRAPHICS_OUTPUT_BLT_PIXEL *CutInHalf(IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Image, IN UINTN Width, IN UINTN Height) {
  UINTN NewWidth = Width / 2;
  UINTN NewHeight = Height / 2;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Result = AllocateZeroPool(NewWidth * NewHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  for (UINTN i = 0; i < Height; ++i) {
    for (UINTN j = 0; j < Width; ++j) {
      if (i % 2 == 0 && j % 2 == 0)
        Result[(i / 2) * NewWidth + (j / 2)] = Image[i * Width + j];
    }
  }
  return Result;
}

EFI_GRAPHICS_OUTPUT_BLT_PIXEL *CombineWithTransparency(IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Foreground,
                                                       IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Background,
                                                       IN UINTN Width, IN UINTN Height) {
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL * Result = AllocateZeroPool(sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * Width * Height);
  for (UINTN y = 0; y < Height; ++y) {
    for (UINTN x = 0; x < Width; ++x) {
      Result[y * Width + x] = Foreground[y * Width + x];
      if (Result[y * Width + x].Reserved != 0xff) {
        Result[y * Width + x] = Background[y * Width + x];
      }
    }
  }
  return Result;
}

