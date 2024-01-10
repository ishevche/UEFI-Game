#include "Common.h"

VOID MyFreePool(IN VOID *Pointer) {
  if (Pointer != NULL)
    FreePool(Pointer);
}

BOOLEAN CheckError(IN EFI_STATUS Status, IN CHAR16 *Location) {
  if (!EFI_ERROR(Status))
    return FALSE;
  gST->ConOut->SetAttribute(gST->ConOut, (EFI_LIGHTRED | EFI_BACKGROUND_BLACK));
  CHAR16 *ErrorStr = (CHAR16 *) L"Error: %r %s\n";
  Print(ErrorStr, Status, Location);
  gST->ConOut->SetAttribute(gST->ConOut, (EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK));
  return TRUE;
}

VOID AddElement(IN OUT VOID ***List, IN OUT UINTN *ListSize, IN VOID *Element) {
  if (*ListSize == 0)
    *List = AllocatePool(sizeof(VOID *) * ((*ListSize) + 1));
  else
    *List = ReallocatePool(sizeof(VOID *) * (*ListSize), sizeof(VOID *) * ((*ListSize) + 1), *List);
  (*List)[*ListSize] = Element;
  (*ListSize)++;
}

