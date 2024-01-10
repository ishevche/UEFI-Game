#include <Uefi.h>
#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <X64/ProcessorBind.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Guid/FileInfo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/Rng.h>
#include <Library/UefiBootManagerLib.h>\

EFI_STATUS ReadFile(IN CHAR16 *filename, OUT CHAR8 **word_list, OUT UINTN *size) {
  EFI_STATUS Status = 0;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
  EFI_FILE_PROTOCOL * Root = NULL;
  EFI_FILE_PROTOCOL * WordsFile = NULL;
  EFI_HANDLE * simple_file_system_handles;
  UINTN handles_num;

  Status = gBS->LocateHandleBuffer(
          ByProtocol,
          &gEfiSimpleFileSystemProtocolGuid,
          NULL,
          &handles_num,
          &simple_file_system_handles
  );
  Print(L"Find handles: %d\n", Status);
  Print(L"Number of handles: %d\n", handles_num);
  Status = gBS->HandleProtocol(
          simple_file_system_handles[1],
          &gEfiSimpleFileSystemProtocolGuid,
          (VOID **) &SimpleFileSystem
  );
  Print(L"Locate FS: %d\n", Status);
  Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root);
  Print(L"Locate Root: %d\n", Status);
  Status = Root->Open(
          Root,
          &WordsFile,
          filename,
          EFI_FILE_MODE_READ,
          0
  );
  Print(L"Locate file: %d\n", Status);
  Status = WordsFile->SetPosition(WordsFile, 0);
  Print(L"Reset file position: %d\n", Status);

  /* Get the file size. */
  UINTN buffSize = 0;
  EFI_FILE_INFO *fileInfo = NULL;
  Status = WordsFile->GetInfo(WordsFile, &gEfiFileInfoGuid, &buffSize, NULL);
  Print(L"Get fileinfo buffer size: %d\n", Status);
  Status = gBS->AllocatePool(EfiLoaderData, buffSize, (VOID **) &fileInfo);
  Print(L"Allocate fileinfo buffer: %d\n", Status);
  Status = WordsFile->GetInfo(WordsFile, &gEfiFileInfoGuid, &buffSize, fileInfo);
  Print(L"Get file info: %d\n", Status);
  *size = fileInfo->FileSize;
  Print(L"File size: %d\n", *size);
  Status = gBS->FreePool(fileInfo);

  Status = gBS->AllocatePool(EfiLoaderData, *size, (VOID **) word_list);
  Print(L"Memory allocation: %d\n", Status);

  for (INTN i = 0; i < *size; ++i) {
    (*word_list)[i] = 0;
  }

  Status = WordsFile->SetPosition(WordsFile, 0);
  Print(L"Reset file position: %d\n", Status);
  Status = WordsFile->Read(WordsFile, size, (*word_list));
  Print(L"Read file: %d\n", Status);
  Status = WordsFile->Close(WordsFile);
  Print(L"Close file: %d\n", Status);
  (*word_list)[(*size) - 1] = 0;
  Status = Root->Close(Root);
  Print(L"Close root: %d\n", Status);
  return Status;
}

EFI_STATUS GetRandom(OUT UINT16 *result) {
  EFI_STATUS Status = 0;
  *result = 10;
  EFI_RNG_PROTOCOL *rngProtocol;
  Status = gBS->LocateProtocol(&gEfiRngProtocolGuid, NULL, (VOID **) &rngProtocol);
  Print(L"Locate rng protocol: %d\n", Status);

//  Status = rngProtocol->GetRNG(rngProtocol, NULL, 1, (UINT8*)result);
//  Print(L"Generate random: %d\n", Status);
//  Print(L"Random value: %d\n", *result);
//  UINTN size = 0;
//  EFI_RNG_ALGORITHM  *RNGAlgorithmList;
//  Status = rngProtocol -> GetInfo(rngProtocol, &size, NULL);
//  Print(L"Get rngProtocol info buffer size: %d\n", Status);
//  Status = gBS -> AllocatePool(EfiLoaderData, size, (VOID **) &RNGAlgorithmList);
//  Print(L"Allocate fileinfo buffer: %d\n", Status);
//  Status = rngProtocol -> GetInfo(rngProtocol, &size, RNGAlgorithmList);
//  Print(L"Get rngProtocol info: %d\n", Status);
//  GetRandomNumber16(result)
  return 0;
}

EFI_STATUS
EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE *SystemTable
) {
  EFI_STATUS Status = EFI_SUCCESS;

  CHAR16 * word_list;
  UINTN length;
  Status = ReadFile(L"words.txt", (CHAR8 * *) & word_list, &length);
  Print(L"File size: %d\n", length);

  UINT16 word_idx;
  Status = GetRandom(&word_idx);

  CHAR16 word[6] = {'\0', '\0', '\0', '\0', '\0', '\0'};
  for (UINTN i = 0; i < 5; ++i) {
    word[i] = word_list[6 * word_idx + i + 1];
  }
  Print(word);
  Print(L"\n");

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
    Print(L"\n");
    CHAR16 reply[6] = {'_', '_', '_', '_', '_', '\0'};
    UINTN pluses = 0;
    for (UINTN idx = 0; idx < 5; ++idx) {
      if (input_word[idx] == word[idx]) {
        reply[idx] = '+';
        pluses++;
      } else {
        for (UINTN idx_2 = 0; idx_2 < 5; ++idx_2) {
          if (input_word[idx] == word[idx_2]) {
            reply[idx] = '?';
            break;
          }
        }
      }
    }
    Print(reply);
    Print(L"\n");
    if (pluses == 5) {
      EFI_BOOT_MANAGER_LOAD_OPTION *BootOption;
      UINTN BootOptionCount;
      BootOption = EfiBootManagerGetLoadOptions(&BootOptionCount, LoadOptionTypeBoot);
      EfiBootManagerBoot(&BootOption[3]);
      break;
    }
  }
  Print(L"Game ended");

  gBS->FreePool(word_list);
  return Status;
}