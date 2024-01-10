#include "FileSystems.h"

#include <Library/FileHandleLib.h>
#include <Library/DevicePathLib.h>

#include <Protocol/LoadedImage.h>

#include <Guid/FileInfo.h>

EFI_HANDLE SelfFSHandle;

EFI_STATUS ReadFile(IN EFI_HANDLE PartitionHandle,
                    IN CHAR16 *Filename,
                    OUT VOID **Data,
                    OUT UINTN *Size) {
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
  EFI_FILE_PROTOCOL * Root = NULL;
  EFI_FILE_PROTOCOL * File = NULL;
  if (Filename == NULL || Data == NULL || Size == NULL) return EFI_INVALID_PARAMETER;

  Status = gBS->HandleProtocol(
          PartitionHandle,
          &gEfiSimpleFileSystemProtocolGuid,
          (VOID **) &SimpleFileSystem
  );
  if (CheckError(Status, (CHAR16 *) L"while getting file system from handle")) return Status;
  Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root);
  if (CheckError(Status, (CHAR16 *) L"while opening the root directory")) return Status;
  Status = Root->Open(Root, &File, Filename, EFI_FILE_MODE_READ, 0);
  if (CheckError(Status, (CHAR16 *) L"while opening the file")) return Status;

  EFI_FILE_INFO *fileInfo = FileHandleGetInfo(File);
  *Size = fileInfo->FileSize;
  MyFreePool(fileInfo);

  *Data = AllocateZeroPool(*Size);

  Status = File->Read(File, Size, (*Data));
  if (CheckError(Status, (CHAR16 *) L"while reading the file")) return Status;
  File->Close(File);
  Root->Close(Root);
  return Status;
}

EFI_STATUS LocateCurFSHandle() {
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_LOADED_IMAGE_PROTOCOL *loaded_image_protocol;
  Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &loaded_image_protocol);
  if (!EFI_ERROR(Status))
    SelfFSHandle = loaded_image_protocol->DeviceHandle;
  return Status;
}

EFI_STATUS LocateFSPartitions(OUT PARTITION **Partitions, OUT UINTN *PartitionsCount) {
  EFI_STATUS Status = EFI_SUCCESS;

  if (Partitions == NULL || PartitionsCount == NULL) return EFI_INVALID_PARAMETER;
  *PartitionsCount = 0;

  UINTN HandleCount;
  EFI_HANDLE * AllHandlesWithFS;
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid,
                                   NULL, &HandleCount, &AllHandlesWithFS);
  if (EFI_ERROR(Status)) return Status;

  for (UINTN HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    EFI_DEVICE_PATH *Path = DevicePathFromHandle(AllHandlesWithFS[HandleIndex]);
    while (Path != NULL && !IsDevicePathEndType(Path)) {
      if (DevicePathType(Path) == MEDIA_DEVICE_PATH && DevicePathSubType(Path) == MEDIA_HARDDRIVE_DP) {
        (*PartitionsCount)++;
        break;
      }
      Path = NextDevicePathNode(Path);
    }
  }

  *Partitions = AllocateZeroPool(*PartitionsCount * sizeof(PARTITION));
  if (*Partitions == NULL) return EFI_OUT_OF_RESOURCES;

  for (UINTN HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    EFI_HANDLE DeviceHandle = AllHandlesWithFS[HandleIndex];
    EFI_DEVICE_PATH *Path = DevicePathFromHandle(DeviceHandle);
    while (Path != NULL && !IsDevicePathEndType(Path)) {
      EFI_DEVICE_PATH *NextDevicePath = NextDevicePathNode(Path);
      if (DevicePathType(Path) == MEDIA_DEVICE_PATH && DevicePathSubType(Path) == MEDIA_HARDDRIVE_DP) {
        HARDDRIVE_DEVICE_PATH *HDPath = (HARDDRIVE_DEVICE_PATH *) Path;
        (*Partitions)[HandleIndex].handle = DeviceHandle;
        (*Partitions)[HandleIndex].guid = (GUID *) HDPath->Signature;
      }
      Path = NextDevicePath;
    }
  }
  return Status;
}
