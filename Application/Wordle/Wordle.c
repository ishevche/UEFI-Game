#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/PrintLib.h>
#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Protocol/SimpleFileSystem.h>

EFI_SYSTEM_TABLE *mgST;
EFI_BOOT_SERVICES *mgBS;
EFI_RUNTIME_SERVICES *mgRT;

EFI_STATUS ReadFile(OUT CHAR16 **word_list, OUT UINTN *size) {
  EFI_STATUS Status = 0;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
  EFI_FILE_PROTOCOL * Root = 0;
  EFI_FILE_PROTOCOL * WordsFile = 0;
  CHAR16 str[31];
  UnicodeSPrint(str, 30, L"Get FS: %d\n", *size);
//  mgST->ConOut->OutputString(mgST->ConOut, str);

  Status = mgBS->LocateProtocol(
          &gEfiSimpleFileSystemProtocolGuid,
          NULL,
          (VOID **) &SimpleFileSystem
  );
  Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root);
  Status = Root->Open(
          Root,
          &WordsFile,
          L"words.txt",
          EFI_FILE_MODE_READ,
          0
  );
  Status = WordsFile->SetPosition(WordsFile, 0);

  /* Get the file size. */
  Status = WordsFile->Read(WordsFile, size, NULL);

  Status = mgBS->AllocatePool(EfiConventionalMemory, *size, (VOID **) word_list);


  (*size) /= sizeof(CHAR16);

  for (INTN i = 0; i < *size; ++i) {
    (*word_list)[i] = 'a';
  }

  (*size) *= sizeof(CHAR16);

  Status = WordsFile->SetPosition(WordsFile, 0);
  Status = WordsFile->Read(WordsFile, size, (*word_list));
  Status = WordsFile->Close(WordsFile);
  (*word_list)[(*size) / sizeof(CHAR16) - 1] = '\0';
  Status = Root->Close(Root);
  return Status;
}

EFI_STATUS GetRandom(OUT UINT8 *result) {
  EFI_STATUS Status = 0;
  *result = 0;
  return Status;
}

EFI_STATUS
EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE *SystemTable
) {
  mgST = SystemTable;
  mgBS = mgST->BootServices;
  mgRT = mgST->RuntimeServices;

  EFI_STATUS Status = EFI_SUCCESS;

  CHAR16 * word_list;
  UINTN length;
  Status = ReadFile(&word_list, &length);

  CHAR16 string[31];
  UnicodeSPrint(string, 30, L"%d\n", length);
  mgST->ConOut->OutputString(mgST->ConOut, string);

  UINT8 word_idx;
  Status = GetRandom(&word_idx);

  CHAR16 word[6] = {'\0', '\0', '\0', '\0', '\0', '\0'};
  for (UINTN i = 0; i < 5; ++i) {
    CHAR16 string[2];
    UnicodeSPrint(string, 2, L"\n");
    word[i] = word_list[6 * word_idx + i + 1];
  }
  mgST->ConOut->OutputString(mgST->ConOut, word);
  mgST->ConOut->OutputString(mgST->ConOut, L"\n");

  for (UINTN move = 0; move < 6; ++move) {
    CHAR16 input_word[6] = {'\0', '\0', '\0', '\0', '\0', '\0'};
    for (UINTN input_idx = 0; input_idx < 5; ++input_idx) {
      UINTN what;
      Status = SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &what);
      EFI_INPUT_KEY Key;
      Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);
      input_word[input_idx] = Key.UnicodeChar;
      SystemTable->ConOut->OutputString(SystemTable->ConOut, &input_word[input_idx]);
    }
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\n");
    CHAR16 reply[6] = {'_', '_', '_', '_', '_', '\0'};
    UINTN pluses = 0;
    for (UINTN idx = 0; idx < 5; ++idx) {
      if (input_word[idx] == word[idx]) {
        reply[idx] = '+';
        pluses ++;
      } else {
        for (UINTN idx_2 = 0; idx_2 < 5; ++idx_2) {
          if (input_word[idx] == word[idx_2]) {
            reply[idx] = '?';
            break;
          }
        }
      }
    }
    SystemTable->ConOut->OutputString(SystemTable->ConOut, reply);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\n");
    if (pluses == 5) {
      break;
    }
  }
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Game ended");

  mgBS->FreePool(word_list);
  return Status;
}