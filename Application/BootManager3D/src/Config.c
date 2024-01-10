#include "Config.h"
#include "FileSystems.h"
#include <Library/MemoryAllocationLib.h>
#include <stdio.h>

EFI_STATUS ParseConfigFile(IN CHAR16 *filename, OUT BOOT_ENTRY **result, OUT UINTN *result_size) {
  EFI_STATUS Status = EFI_SUCCESS;
  CHAR8 * FileData;
  UINTN length;
  ReadFile(SelfFSHandle, filename, (VOID **) &FileData, &length);
  CHAR8 prev = '\0';
  *result_size = 1;
  for (UINTN i = 0; i < length; ++i) {
    CHAR8 cur = FileData[i];
    if (cur == '\n' && prev == '\n') {
      (*result_size)++;
    }
    prev = cur;
  }
  *result = AllocateZeroPool(*result_size * sizeof(BOOT_ENTRY));
  UINTN cur_struct = 0;
  UINTN cur_value = 0;
  prev = '\0';
  UINTN cur_char = 0;
  BOOLEAN write = FALSE;
  for (UINTN i = 0; i < length; ++i) {
    CHAR8 cur = FileData[i];
    if (cur == '\n' && prev == '\n') {
      cur_struct++;
      cur_value = 0;
    } else if (cur == '\n') {
      write = FALSE;

      CHAR8 * cur_elem = AllocatePool((cur_char + 1) * sizeof(CHAR8));
      for (UINTN j = 0; j < cur_char; ++j) {
        cur_elem[j] = FileData[i - cur_char + j];
      }
      cur_elem[cur_char] = '\0';
      CHAR16 * dest = AllocatePool((cur_char + 1) * sizeof(CHAR16));
      Status = AsciiStrToUnicodeStrS(cur_elem, dest, cur_char + 1);
      cur_char = 0;
      switch (cur_value) {
        case 0:
          (*result)[cur_struct].title = dest;
          break;
        case 1:
          (*result)[cur_struct].volume_guid = dest;
          break;
        case 2:
          (*result)[cur_struct].loader_name = dest;
          break;
        case 3:
          (*result)[cur_struct].icon = dest;
          break;
        case 4:
          (*result)[cur_struct].options = dest;
          break;
        default:
          break;
      }
      cur_value++;
    } else if (cur == ' ' && prev == ':') {
      write = TRUE;
    } else {
      if (write) {
        cur_char++;
      }
    }
    prev = cur;
  }
  return Status;
}

EFI_STATUS ParseTexturesConfigFile(IN CHAR16 *Filename, OUT CHAR16 ***TexturesFiles, OUT UINTN *TexturesCount) {
  if (Filename == NULL || TexturesFiles == NULL || TexturesCount == NULL) return EFI_INVALID_PARAMETER;
  EFI_STATUS Status = EFI_SUCCESS;
  CHAR8 * FileData;
  UINTN FileLength;
  Status = ReadFile(SelfFSHandle, Filename, (VOID **) &FileData, &FileLength);
  if (CheckError(Status, (CHAR16 *) L"while reading textures config")) return Status;
  *TexturesCount = 0;
  for (UINTN i = 0; i < FileLength; ++i)
    if (FileData[i] == '\n')
      (*TexturesCount)++;
  *TexturesFiles = AllocateZeroPool(sizeof(CHAR16 *) * (*TexturesCount));
  CHAR8 * CurFileNameStart = FileData;
  UINTN CurTexture = 0;
  for (UINTN i = 0; i < FileLength; ++i)
    if (FileData[i] == '\n') {
      FileData[i] = '\0';
      UINTN NameSize = AsciiStrLen(CurFileNameStart) + 1;
      (*TexturesFiles)[CurTexture] = AllocateZeroPool(NameSize * sizeof(CHAR16));
      AsciiStrToUnicodeStrS(CurFileNameStart, (*TexturesFiles)[CurTexture], NameSize);
      CurTexture++;
      CurFileNameStart = FileData + i + 1;
    }
  MyFreePool(FileData);
  return Status;
}

EFI_STATUS ParseMazeConfigFile(IN CHAR16 *Filename, OUT INTN **Numbers, OUT UINTN *Width, OUT UINTN *Height) {
  if (Filename == NULL || Numbers == NULL || Width == NULL || Height == NULL) return EFI_INVALID_PARAMETER;
  EFI_STATUS Status = EFI_SUCCESS;
  CHAR8 * FileData;
  UINTN FileLength;
  Status = ReadFile(SelfFSHandle, Filename, (VOID **) &FileData, &FileLength);
  if (CheckError(Status, (CHAR16 *) L"while reading map config")) return Status;
  *Height = 0;
  *Width = -1;
  INTN CurWidth = 1;
  for (UINTN i = 0; i < FileLength; ++i) {
    if (FileData[i] == ' ')
      CurWidth++;
    else if (FileData[i] == '\n') {
      (*Height)++;
      if (*Width == -1)
        *Width = CurWidth;
      else if (*Width != CurWidth)
        Print((CHAR16 *) L"Row #%d has different amount of elements\n", *Height);
      CurWidth = 1;
    }
  }
  *Numbers = AllocateZeroPool((*Width) * (*Height) * sizeof(INTN));
  CHAR8* CurBuffer = FileData;
  for (UINTN i = 0; i < *Width * *Height; ++i) {
    INT32 Read;
    if (sscanf(CurBuffer, "%lld%n", &(*Numbers)[i], &Read) != 1) {
      Print((CHAR16 *) L"Error occurred while reading maze config: end of data reached to early\n");
      return EFI_INVALID_PARAMETER;
    }
    CurBuffer += Read;
  }
  return Status;
}
