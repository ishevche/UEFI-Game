#ifndef OSPROJECT_COMMON_H
#define OSPROJECT_COMMON_H

#include <Uefi.h>
#include <X64/ProcessorBind.h>

#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

BOOLEAN CheckError(IN EFI_STATUS Status, IN CHAR16 *Location);

VOID MyFreePool(IN VOID *Pointer);

VOID AddElement(IN OUT VOID ***List, IN OUT UINTN *ListSize, IN VOID *Element);

#endif //OSPROJECT_COMMON_H
