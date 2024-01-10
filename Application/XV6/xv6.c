#include "xv6.h"
#include "elf.h"
#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <X64/ProcessorBind.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Protocol/BlockIo.h>
#include  <Protocol/LoadedImage.h>
#include  <Protocol/SimpleFileSystem.h>
#include  <Library/DevicePathLib.h>
#include  <Guid/FileInfo.h>
#include  <Guid/Acpi.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Library/BaseMemoryLib.h>


EFI_STATUS LoadFile(CHAR16 *FileName, EFI_PHYSICAL_ADDRESS *FileAddr, UINTN *FilePageSize) {
  EFI_STATUS Status;
  EFI_FILE_PROTOCOL * Root;
  EFI_FILE_PROTOCOL * File;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFile;

  Status = gBS->LocateProtocol(
          &gEfiSimpleFileSystemProtocolGuid,
          NULL,
          (VOID **) &SimpleFile
  );
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Locate Simple File System Protocol.\n");
    return Status;
  }

  Status = SimpleFile->OpenVolume(SimpleFile, &Root);
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Open Volume.\n");
    return Status;
  }
  Print(L"Loading File: %s\n", FileName);

  Status = Root->Open(Root, &File, FileName, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Open %s\n", FileName);
    return Status;
  }

  UINTN FileInfoBufferSize = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * StrLen(FileName) + 2;
  UINT8 FileInfoBuffer[FileInfoBufferSize];
  Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoBufferSize, FileInfoBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"Failed to Get FileInfo\n");
    return Status;
  }

  EFI_FILE_INFO *FileInfo = (EFI_FILE_INFO *) FileInfoBuffer;
  UINTN FileSize = FileInfo->FileSize;
  Print(L"FileSize=%d\n", FileSize);
  *FilePageSize = (FileSize + 4095) / 4096;
  *FileAddr = 0;
  Status = gBS->AllocatePages(
          AllocateAnyPages,
          EfiLoaderData,
          *FilePageSize,
          FileAddr);
  if (EFI_ERROR(Status)) {
    Print(L"Failed to Allocate Pages\n");
    return Status;
  }

  Status = File->Read(
          File,
          &FileSize,
          (VOID *) *FileAddr
  );
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Load File\n");
    return Status;
  }
  Print(L"Successfully Loaded File: %s\n", FileName);

  return Status;
}


EFI_STATUS
EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE *SystemTable
) {
  asm("cli");
  goto mylabel;
mylabel:
  EFI_STATUS Status = EFI_SUCCESS;
  CHAR16 * kernel_path = L"\\kernel";
  EFI_PHYSICAL_ADDRESS parameters_address = 0x50000;
  EFI_PHYSICAL_ADDRESS entry_address;
  Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, 1, &parameters_address);
  Print(L"Allocate boot parameters status: %d\r\n", Status);
  struct boot *boot = (struct boot *) parameters_address;

  EFI_PHYSICAL_ADDRESS buffer_address;
  UINTN buffer_page_size;
  Status = LoadFile(kernel_path, &buffer_address, &buffer_page_size);
  Print(L"Kernel load status: %d\r\n", Status);
  elfhdr *elf = (elfhdr *) buffer_address;
  proghdr *ph = (proghdr *) ((UINT8 *) elf + elf->phoff);
  proghdr *eph = ph + elf->phnum;

  EFI_PHYSICAL_ADDRESS pages_start = ph->paddr;
  Print(L"Page start before for: %x\n", pages_start);
  UINTN mask = ph->align - 1;
  EFI_PHYSICAL_ADDRESS pages_end = pages_start + ph->memsz;
  pages_end = (pages_end + mask) & ~mask;
  for (; ph < eph; ph++) {
    if (ph->type != 1) {
      continue;
//    Print(L"----------\n");
//    Print(L"type    : %x\n", ph->type);
//    Print(L"off     : %x\n", ph->off);
//    Print(L"vaddr   : %x\n", ph->vaddr);
//    Print(L"paddr   : %x\n", ph->paddr);
//    Print(L"filesz  : %x\n", ph->filesz);
//    Print(L"memsz   : %x\n", ph->memsz);
//    Print(L"flags   : %x\n", ph->flags);
//    Print(L"align   : %x\n", ph->align);
//    Print(L"size    : %d\n", sizeof(proghdr));
//    Print(L"----------\n");
      EFI_PHYSICAL_ADDRESS cur_start = ph->paddr;
      EFI_PHYSICAL_ADDRESS cur_end = cur_start + ph->memsz;
      UINTN mask = ph->align - 1;
      cur_end = (cur_end + mask) & ~mask;
      if (cur_start < pages_start) {
        pages_start = cur_start;
      }
      if (cur_end > pages_end) {
        pages_end = cur_end;
      }
    }
    Print(L"\n");
  }
  UINTN page_size = ((pages_end - pages_start) / 4096) + 1;
  Print(L"Page start after for: %x\n", pages_start);
  Print(L"Page end after for: %x\n", pages_end);
  Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, page_size, &pages_start);
  Print(L"Allocate pages status: %d\r\n", Status);

  ph = (proghdr *) ((UINT8 *) elf + elf->phoff);
  for (; ph < eph; ph++) {
    if(ph->type != 1)
      continue;
    EFI_PHYSICAL_ADDRESS cur_start = ph->paddr;
    EFI_PHYSICAL_ADDRESS cur_end = cur_start + ph->memsz;
    UINTN mask = ph->align - 1;
    cur_end = (cur_end + mask) & ~mask;
    CopyMem((VOID *) cur_start, (VOID *) (buffer_address + ph->off), ph->filesz);
    if (ph->memsz > ph->filesz) {
      SetMem((VOID *) (cur_start + ph->filesz), ph->memsz - ph->filesz, 0);
    }
  }

  Print(L"aaaaaaaaa    : %llx\n", (EFI_PHYSICAL_ADDRESS) elf->entry);

  entry_address = (EFI_PHYSICAL_ADDRESS) elf->entry;
  Print(L"elf.entry    : %llx\n", elf->entry);
  Print(L"cast         : %llx\n", (EFI_PHYSICAL_ADDRESS) elf->entry);
  Print(L"entry_address: %llx\n", entry_address);
  FreePages((VOID *) buffer_address, buffer_page_size);

  boot->entry = (UINT64) entry_address;
  boot->entry_addr = entry_address;
  boot->gdt_desc->size = sizeof(struct gdt) * 3 - 1;
  boot->gdt_desc->gdt_addr = (UINT64) boot->gdt;
  boot->gdt[0].limit = 0;
  boot->gdt[0].base1 = 0;
  boot->gdt[0].base2 = 0;
  boot->gdt[0].access_byte = 0;
  boot->gdt[0].flags = 0;
  boot->gdt[0].base3 = 0;

  boot->gdt[1].limit = (0xffffffff >> 12) & 0xFFFF;
  boot->gdt[1].base1 = 0x0 & 0xFFFF;
  boot->gdt[1].base2 = (0x0 >> 16) & 0xFF;
  boot->gdt[1].access_byte = 0x90 | STA_X | STA_R;
  boot->gdt[1].flags = 0xC0 | ((0xffffffff >> 28) & 0xF);
  boot->gdt[1].base3 = (0x0 >> 24) & 0xFF;

  boot->gdt[2].limit = (0xffffffff >> 12) & 0xFFFF;
  boot->gdt[2].base1 = 0x0 & 0xFFFF;
  boot->gdt[2].base2 = (0x0 >> 16) & 0xFF;
  boot->gdt[2].access_byte = 0x90 | STA_W;
  boot->gdt[2].flags = 0xC0 | ((0xffffffff >> 28) & 0xF);
  boot->gdt[2].base3 = (0x0 >> 24) & 0xFF;

  struct MemoryMap MemoryMap = {NULL, 0, 0, 0, 0};
  Status = gBS->GetMemoryMap(
          &MemoryMap.MapSize,
          (EFI_MEMORY_DESCRIPTOR *) MemoryMap.Buffer,
          &MemoryMap.MapKey,
          &MemoryMap.DescriptorSize,
          &MemoryMap.DescriptorVersion);
  Status = gBS->AllocatePool(EfiLoaderData, MemoryMap.MapSize, &MemoryMap.Buffer);
  Status = gBS->GetMemoryMap(
          &MemoryMap.MapSize,
          (EFI_MEMORY_DESCRIPTOR *) MemoryMap.Buffer,
          &MemoryMap.MapKey,
          &MemoryMap.DescriptorSize,
          &MemoryMap.DescriptorVersion);
  Status = gBS->ExitBootServices(ImageHandle, MemoryMap.MapKey);
  asm("movl $0x7C00, %esp");
  asm("lgdt %0": : "m"(boot->gdt_desc));
  asm("push $0x08");
  asm("push %0": : "m"(boot->entry_addr));
  asm("lretq");
  while (1);
}
