#include "DemoGame.h"
#include "BMPReader.h"
#include "FileSystems.h"
#include "Graphics.h"
#include <X64/ProcessorBind.h>
#include <Uefi/UefiBaseType.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/GraphicsOutput.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


#define texWidth 64
#define texHeight 64
#define floorNum 0
#define ceilNum 1
#define moveSpeed 0.05
#define rotSpeed 0.1
#define escapes 12

double abs_d(double a) {
  if (a < 0)
    return -1 * a;
  return a;
}


EFI_STATUS MakeBootEntryTexture(IN BOOT_ENTRY BootEntry,
                                IN UINTN BaseTextureIndex,
                                IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL ***Textures,
                                IN OUT UINTN *TexturesCount) {
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL * BootEntryIcon;
  INT32 Width, Height;
  Status = ReadBMP(SelfFSHandle, BootEntry.icon, &Width, &Height, &BootEntryIcon);
  CheckError(Status, CatSPrint(NULL, (const CHAR16 *) L"reading BMP %s", BootEntry.icon));
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL * BootEntryTexture;
  BootEntryTexture = CombineWithTransparency((*Textures)[BaseTextureIndex], BootEntryIcon, Width, Height);
  AddElement((VOID ***) Textures, TexturesCount, BootEntryTexture);
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
            Print(L"Texture #%d is %s\n", *TexturesCount, BootEntries[i].title);
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
           BOOT_ENTRY *BootEntries, UINTN BootEntriesCount) {
  UINTN bootable_start = TexturesCount;

  PlaceBootEntries(Map, MapWidth, MapHeight, &Textures, &TexturesCount, BootEntries, BootEntriesCount);

  EFI_GRAPHICS_OUTPUT_PROTOCOL *graphicsProtocol;


  gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **) &graphicsProtocol);
  UINTN screenWidth = graphicsProtocol->Mode->Info->HorizontalResolution;
  UINTN screenHeight = graphicsProtocol->Mode->Info->VerticalResolution;


  EFI_GRAPHICS_OUTPUT_BLT_PIXEL * screen =
          AllocateZeroPool(sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * screenWidth * screenHeight);
  double posX = 1.5, posY = 17.5;
  double dirX = 1.0, dirY = 0.0;
  double planeX = 0.0, planeY = 0.66;


  BOOLEAN ready_to_boot = FALSE;
  UINTN EntryToBoot = ESCAPE_ENTRY;
  while (!ready_to_boot) {
    for (int y = 0; y < screenHeight; y++) {
      double rayDirX0 = dirX - planeX;
      double rayDirY0 = dirY - planeY;
      double rayDirX1 = dirX + planeX;
      double rayDirY1 = dirY + planeY;

      int p = y - screenHeight / 2;

      double posZ = 0.5 * screenHeight;

      double rowDistance = posZ / p;

      double floorStepX = rowDistance * (rayDirX1 - rayDirX0) / screenWidth;
      double floorStepY = rowDistance * (rayDirY1 - rayDirY0) / screenWidth;

      double floorX = posX + rowDistance * rayDirX0;
      double floorY = posY + rowDistance * rayDirY0;

      for (UINT16 x = 0; x < screenWidth; ++x) {
        int cellX = (int) (floorX);
        int cellY = (int) (floorY);

        int tx = (int) (texWidth * (floorX - cellX)) & (texWidth - 1);
        int ty = (int) (texHeight * (floorY - cellY)) & (texHeight - 1);

        floorX += floorStepX;
        floorY += floorStepY;

        EFI_GRAPHICS_OUTPUT_BLT_PIXEL color;

        color = Textures[floorNum][ty * texWidth + tx];
        color.Blue /= 2;
        color.Green /= 2;
        color.Red /= 2;
        screen[y * screenWidth + x] = color;

        color = Textures[ceilNum][ty * texWidth + tx];
        color.Blue /= 2;
        color.Green /= 2;
        color.Red /= 2;
        screen[(screenHeight - y - 1) * screenWidth + x] = color;
      }
    }
    for (int x = 0; x < screenWidth; x++) {
      double cameraX = 2 * x / (double) screenWidth - 1;
      double rayDirX = dirX + planeX * cameraX;
      double rayDirY = dirY + planeY * cameraX;
      int mapX = (int) posX;
      int mapY = (int) posY;
      double sideDistX;
      double sideDistY;

      double deltaDistX = (rayDirX == 0) ? MAX_UINT32 : abs_d(1 / rayDirX);
      double deltaDistY = (rayDirY == 0) ? MAX_UINT32 : abs_d(1 / rayDirY);

      double perpWallDist;

      int stepX;
      int stepY;

      int hit = 0;
      int side;

      if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
      } else {
        stepX = 1;
        sideDistX = (mapX + 1.0 - posX) * deltaDistX;
      }
      if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
      } else {
        stepY = 1;
        sideDistY = (mapY + 1.0 - posY) * deltaDistY;
      }

      while (hit == 0) {
        if (sideDistX < sideDistY) {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        } else {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        if (Map[mapY * MapWidth + mapX] > 0) hit = 1;
      }
      if (side == 0) perpWallDist = (sideDistX - deltaDistX);
      else perpWallDist = (sideDistY - deltaDistY);

      int lineHeight = (int) (screenHeight / perpWallDist);

      int drawStart = -lineHeight / 2 + screenHeight / 2;
      if (drawStart < 0) drawStart = 0;
      int drawEnd = lineHeight / 2 + screenHeight / 2;
      if (drawEnd >= screenHeight) drawEnd = screenHeight - 1;

      int texNum = Map[mapY * MapWidth + mapX];

      double wallX;
      if (side == 0) wallX = posY + perpWallDist * rayDirY;
      else wallX = posX + perpWallDist * rayDirX;
      wallX -= floor((wallX));

      int texX = (int) (wallX * (double) (texWidth));
      if (side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
      if (side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

      double step = 1.0 * texHeight / lineHeight;
      // Starting texture coordinate
      double texPos = (drawStart - (double) screenHeight / 2 + (double) lineHeight / 2) * step;
      for (int y = drawStart; y < drawEnd; y++) {
        int texY = (int) texPos & (texHeight - 1);
        texPos += step;
        EFI_GRAPHICS_OUTPUT_BLT_PIXEL color = Textures[texNum][texY * texWidth + texX];
        if (side == 1) {
          color.Blue /= 2;
          color.Green /= 2;
          color.Red /= 2;
        }
        screen[y * screenWidth + x] = color;
      }
    }

    graphicsProtocol->Blt(
            graphicsProtocol,
            screen,
            EfiBltBufferToVideo,
            0, 0,
            0, 0,
            screenWidth, screenHeight,
            screenWidth * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
    );

    UINTN what;
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &what);
    EFI_INPUT_KEY Key;
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    double oldDirX;
    double oldPlaneX;
    INTN nextX;
    INTN nextY;
    switch (Key.ScanCode) {
      case SCAN_UP:
        nextY = Map[(int) (posY + dirY * (moveSpeed + 0.05)) * MapWidth + (int) (posX)];
        nextX = Map[(int) (posY) * MapWidth + (int) (posX + dirX * (moveSpeed + 0.05))];
        if (nextY == 0 || nextY >= bootable_start) posY += dirY * moveSpeed;
        if (nextX == 0 || nextX >= bootable_start) posX += dirX * moveSpeed;
        break;
      case SCAN_DOWN:
        nextY = Map[(int) (posY - dirY * (moveSpeed + 0.05)) * MapWidth + (int) (posX)];
        nextX = Map[(int) (posY) * MapWidth + (int) (posX - dirX * (moveSpeed + 0.05))];
        if (nextY == 0 || nextY >= bootable_start) posY -= dirY * moveSpeed;
        if (nextX == 0 || nextX >= bootable_start) posX -= dirX * moveSpeed;
        break;
      case SCAN_LEFT:
        oldDirX = dirX;
        dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
        dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
        oldPlaneX = planeX;
        planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
        planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
        break;
      case SCAN_RIGHT:
        oldDirX = dirX;
        dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
        dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
        oldPlaneX = planeX;
        planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
        planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
        break;
      case SCAN_ESC:
        ready_to_boot = TRUE;
        break;
      default:
        break;
    }
    if (Map[(int) (posY) * MapWidth + (int) (posX)] >= bootable_start) {
      EntryToBoot = Map[(int) (posY) * MapWidth + (int) (posX)] - bootable_start;
      ready_to_boot = TRUE;
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