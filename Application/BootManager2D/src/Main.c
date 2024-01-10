#include "Common.h"
#include "Config.h"
#include "FileSystems.h"
#include "BMPReader.h"
#include "Graphics.h"
#include "DemoGame.h"
#include <time.h>
#include <stdlib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/LoadedImage.h>
#include <stdio.h>

#define BOOT_OPTION 1

EFI_STATUS BootEntry(IN BOOT_ENTRY boot_entry, IN PARTITION *partitions, IN UINTN partitions_count) {
  EFI_STATUS Status = EFI_SUCCESS;
  for (UINTN i = 0; i < partitions_count; ++i) {
    GUID entry_guid;
    StrToGuid(boot_entry.volume_guid, &entry_guid);
    if (CompareGuid(&entry_guid, partitions[i].guid)) {
      EFI_HANDLE child_handle;
      Status = gBS->LoadImage(FALSE, gImageHandle,
                              FileDevicePath(partitions[i].handle, boot_entry.loader_name),
                              NULL, 0, &child_handle);
      if (CheckError(Status, CatSPrint(NULL, (CHAR16 *) L"while loading %s", boot_entry.title)))
        return Status;
      EFI_LOADED_IMAGE_PROTOCOL *child_loaded_image;
      gBS->HandleProtocol(child_handle, &gEfiLoadedImageProtocolGuid,
                          (VOID **) &child_loaded_image);
      child_loaded_image->LoadOptions = boot_entry.options;
      child_loaded_image->LoadOptionsSize = boot_entry.options ?
                                            (StrLen(boot_entry.options) + 1) * sizeof(CHAR16) : 0;
      gBS->StartImage(child_handle, NULL, NULL);
      break;
    }
  }
  return Status;
}

EFI_STATUS EFIAPI UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE *SystemTable
) {
  srand(time(0));
  EFI_STATUS Status = EFI_SUCCESS;

  Status = LocateCurFSHandle();
  if (CheckError(Status, L"while getting current file system")) return Status;

  UINTN BootEntriesCount = 0;
  BOOT_ENTRY *BootEntries;
  Status = ParseConfigFile((CHAR16 *) L"config.txt", &BootEntries, &BootEntriesCount);
  if (CheckError(Status, (CHAR16 *) L"while parsing config file")) return Status;

  UINTN HandlesCount;
  PARTITION *partitions;
  Status = LocateFSPartitions(&partitions, &HandlesCount);
  if (CheckError(Status, (CHAR16 *) L"while locating fs partitions")) return Status;

  INTN *Numbers;
  UINTN MapWidth, MapHeight;
  ParseMazeConfigFile((CHAR16 *) L"MazeConfig.conf", &Numbers, &MapWidth, &MapHeight);

  CHAR16 * *TexturesFiles;
  UINTN TexturesCount;
  ParseTexturesConfigFile((CHAR16 *) L"TexturesConfig.conf", &TexturesFiles, &TexturesCount);
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL * *Textures = AllocateZeroPool(TexturesCount * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL *));
  for (UINTN i = 0; i < TexturesCount; ++i) {
    INT32 Width, Height;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Texture;
    Status = ReadBMP(SelfFSHandle, TexturesFiles[i], &Width, &Height, &Texture);
    if (Width != 64 || Height != 64)
      CheckError(EFI_INVALID_PARAMETER, (CHAR16 *) L"wrong texture size");
    if (CheckError(Status, CatSPrint(NULL, L"reading BMP file %s", TexturesFiles[i]))) return Status;
    Textures[i] = CutInHalf(Texture, Width, Height);
    MyFreePool(Texture);
  }
  Print(L"3D Done\n");

  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Sprite;
  UINTN Width, Height;
  Status = ParseSprite(L"Sprite.txt", &Sprite, &Width, &Height);
  if (CheckError(Status, L"reading actor file")) return Status;
  Print(L"Sprite done\n");

  UINTN EntryToBoot = Game(Numbers, MapWidth, MapHeight, Textures, TexturesCount, BootEntries,
                           BootEntriesCount, Sprite);

  if (EntryToBoot != ESCAPE_ENTRY) {
    Status = BootEntry(BootEntries[EntryToBoot], partitions, HandlesCount);
  }
  return Status;
}
