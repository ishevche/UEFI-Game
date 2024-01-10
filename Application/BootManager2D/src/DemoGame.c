#include "DemoGame.h"
#include "BMPReader.h"
#include "FileSystems.h"
#include "Graphics.h"
#include <stdlib.h>
#include <Protocol/GraphicsOutput.h>


#define texWidth 64
#define texHeight 64
#define floorNum 0
#define ceilNum 1
#define moveSpeed 0.05
#define rotSpeed 0.1
#define escapes 12

EFI_STATUS MakeBootEntryTexture(IN BOOT_ENTRY BootEntry,
                                IN UINTN BaseTextureIndex,
                                IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL ***Textures,
                                IN OUT UINTN *TexturesCount) {
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL * BootEntryIcon, *SmallBootEntryIcon;
  INT32 Width, Height;
  Status = ReadBMP(SelfFSHandle, BootEntry.icon, &Width, &Height, &BootEntryIcon);
  CheckError(Status, CatSPrint(NULL, (const CHAR16 *) L"reading BMP %s", BootEntry.icon));
  SmallBootEntryIcon = CutInHalf(BootEntryIcon, Width, Height);
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL * BootEntryTexture;
  BootEntryTexture = CombineWithTransparency((*Textures)[BaseTextureIndex], SmallBootEntryIcon, Width / 2, Height / 2);
  AddElement((VOID ***) Textures, TexturesCount, BootEntryTexture);
  FreePool(BootEntryIcon);
  FreePool(SmallBootEntryIcon);
  return Status;
}

VOID PlaceBootEntries(INTN *Map, UINTN MapWidth, UINTN MapHeight,
                      EFI_GRAPHICS_OUTPUT_BLT_PIXEL ***Textures, UINTN *TexturesCount,
                      BOOT_ENTRY *BootEntries, UINTN BootEntriesCount) {
  UINTN bootable_start = *TexturesCount;
  for (UINTN i = 0; i < BootEntriesCount; ++i) {
    UINTN rand_index = rand() % (escapes - i);
    UINTN escape_count = 0;
    for (UINTN x = 0; x < MapWidth; ++x) {
      for (UINTN y = 0; y < MapHeight; ++y) {
        INTN cell = Map[y * MapWidth + x];
        if (cell >= 10 && cell < bootable_start) {
          if (escape_count == rand_index) {
            Map[y * MapWidth + x] = *TexturesCount;
            MakeBootEntryTexture(BootEntries[i], cell, Textures, TexturesCount);
            escape_count++;
          } else {
            escape_count++;
          }
        }
      }
    }
  }
}


UINTN Game(INTN *Map, UINTN MapWidth, UINTN MapHeight,
           EFI_GRAPHICS_OUTPUT_BLT_PIXEL **Textures, UINTN TexturesCount,
           BOOT_ENTRY *BootEntries, UINTN BootEntriesCount,
           EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Sprite) {
  UINTN bootable_start = TexturesCount;

  PlaceBootEntries(Map, MapWidth, MapHeight, &Textures, &TexturesCount, BootEntries, BootEntriesCount);

  Print(L"Embeding boot entries: done\n");
  EFI_GRAPHICS_OUTPUT_PROTOCOL *graphicsProtocol;

  gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **) &graphicsProtocol);
  UINTN screenWidth = graphicsProtocol->Mode->Info->HorizontalResolution;
  UINTN screenHeight = graphicsProtocol->Mode->Info->VerticalResolution;
  UINTN cell_side = 32;
  UINTN width_margin = (screenWidth - cell_side * MapWidth) / 2;
  UINTN height_margin = (screenHeight - cell_side * MapHeight) / 2;

  UINTN CurX = 1;
  UINTN CurY = 17;

  for (UINTN y = 0; y < MapHeight; ++y) {
    for (UINTN x = 0; x < MapWidth; ++x) {
      EFI_GRAPHICS_OUTPUT_BLT_PIXEL * CurCell = Textures[Map[y * MapWidth + x]];
      if (x == CurX && y == CurY)
        CurCell = CombineWithTransparency(Sprite, CurCell, cell_side, cell_side);
      graphicsProtocol->Blt(
              graphicsProtocol,
              CurCell,
              EfiBltBufferToVideo,
              0, 0,
              x * cell_side + width_margin,
              y * cell_side + height_margin,
              cell_side, cell_side,
              cell_side * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
      );
      if (x == CurX && y == CurY)
        MyFreePool(CurCell);
    }
  }

  BOOLEAN Escaped = FALSE;
  UINTN EntryToBoot = ESCAPE_ENTRY;
  while (!Escaped) {
    UINTN what;
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &what);
    EFI_INPUT_KEY Key;
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    UINTN NewX = CurX;
    UINTN NewY = CurY;
    UINTN MapValue;
    switch (Key.ScanCode) {
      case SCAN_UP:
        if (CurY == 0) break;
        MapValue = Map[(CurY - 1) * MapWidth + CurX];
        if (MapValue != 0 && MapValue < bootable_start) break;
        NewY -= 1;
        break;
      case SCAN_DOWN:
        if (CurY == MapHeight - 1) break;
        MapValue = Map[(CurY + 1) * MapWidth + CurX];
        if (MapValue != 0 && MapValue < bootable_start) break;
        NewY += 1;
        break;
      case SCAN_LEFT:
        if (CurX == 0) break;
        MapValue = Map[CurY * MapWidth + CurX - 1];
        if (MapValue != 0 && MapValue < bootable_start) break;
        NewX -= 1;
        break;
      case SCAN_RIGHT:
        if (CurX == MapWidth - 1) break;
        MapValue = Map[CurY * MapWidth + CurX + 1];
        if (MapValue != 0 && MapValue < bootable_start) break;
        NewX += 1;
        break;
      case SCAN_ESC:
        Escaped = TRUE;
        break;
      default:
        break;
    }
    if (NewX != CurX || NewY != CurY) {
      graphicsProtocol->Blt(
              graphicsProtocol,
              Textures[Map[CurY * MapWidth + CurX]],
              EfiBltBufferToVideo,
              0, 0,
              CurX * cell_side + width_margin,
              CurY * cell_side + height_margin,
              cell_side, cell_side,
              cell_side * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
      );
      EFI_GRAPHICS_OUTPUT_BLT_PIXEL * CombinedSprite =
              CombineWithTransparency(Sprite, Textures[Map[NewY * MapWidth + NewX]], 32, 32);
      graphicsProtocol->Blt(
              graphicsProtocol,
              CombinedSprite,
              EfiBltBufferToVideo,
              0, 0,
              NewX * cell_side + width_margin,
              NewY * cell_side + height_margin,
              cell_side, cell_side,
              cell_side * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
      );
      CurX = NewX;
      CurY = NewY;
    }
    if (MapValue >= bootable_start) {
      EntryToBoot = MapValue - bootable_start;
      Escaped = TRUE;
    }
  }

  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black = {0, 0, 0, 0};

  graphicsProtocol->Blt(
          graphicsProtocol,
          &Black,
          EfiBltVideoFill,
          0, 0, 0, 0,
          screenWidth, screenHeight, 0
  );
  return EntryToBoot;
}