#include <Uefi.h>
#include <Uefi/UefiBaseType.h>
#include <Pi/PiFirmwareFile.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/RngLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/UefiLib.h>
#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/Rng.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadFile.h>

typedef struct Volume {
    EFI_HANDLE VolumeHandle;
    EFI_GUID Guid;
} Volume;

extern Volume *file_systems;
extern UINTN file_systems_count;

VOID MyFreePool(IN VOID *Pointer) {
  if (Pointer != NULL)
    FreePool(Pointer);
}

VOID PrintGuid(IN VOID *Input) {
  GUID * GUID = Input;
  Print(L"%x-%x-%x-%x%x-%x%x%x%x%x%x\n", GUID->Data1, GUID->Data2, GUID->Data3,
        GUID->Data4[0], GUID->Data4[1], GUID->Data4[2], GUID->Data4[3],
        GUID->Data4[4], GUID->Data4[5], GUID->Data4[6], GUID->Data4[7]);
}

EFI_STATUS
EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE *SystemTable
) {
  EFI_STATUS Status = EFI_SUCCESS;

  EFI_HANDLE * AllHandles, DeviceHandle;
  UINTN HandlesCount;

  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &HandlesCount, &AllHandles);
  Print(L"Locate Handles status: %d\r\n", Status);
  Print(L"%d handles located\r\n", HandlesCount);

  for (UINTN HandleIndex = 0; HandleIndex < HandlesCount; HandleIndex++) {
    Print(L"--------------\r\nProcessing handle %d\r\n", HandleIndex);
    DeviceHandle = AllHandles[HandleIndex];

    EFI_DEVICE_PATH *Path = DuplicateDevicePath(DevicePathFromHandle(DeviceHandle));
    while (Path != NULL && !IsDevicePathEndType(Path)) {
      EFI_DEVICE_PATH *NextDevicePath = NextDevicePathNode(Path);
      Print(L"Path type: %x", Path->Type);
      Print(L".%x\n", Path->SubType);
      if (DevicePathType(Path) == MEDIA_DEVICE_PATH && DevicePathSubType(Path) == MEDIA_HARDDRIVE_DP) {
        HARDDRIVE_DEVICE_PATH *HDPath = (HARDDRIVE_DEVICE_PATH *)Path;
        Print(L"GUID: ");
        PrintGuid(HDPath->Signature);
      }
      Path = NextDevicePath;
    }
    EFI_FILE *Root;
    EFI_FILE_SYSTEM_INFO *FileSystemInfo = NULL;
    UINTN Size = 0;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
    Status = gBS->HandleProtocol(
            DeviceHandle,
            &gEfiSimpleFileSystemProtocolGuid,
            (VOID *) &Volume
    );
    Print(L"Get FS protocol status: %d\r\n", Status);
    Status = Volume->OpenVolume(
            Volume,
            &Root
    );
    Print(L"Open volume status: %d\r\n", Status);
    Status = Root->GetInfo(Root, &gEfiFileSystemInfoGuid, &Size, FileSystemInfo);
    FileSystemInfo = AllocateZeroPool(Size);
    Status = Root->GetInfo(Root, &gEfiFileSystemInfoGuid, &Size, FileSystemInfo);
    Print(L"Get FS info status: %d\r\n", Status);
    Print(L"Volume Label: %s\r\n", FileSystemInfo->VolumeLabel);
  }

  return Status;
}
