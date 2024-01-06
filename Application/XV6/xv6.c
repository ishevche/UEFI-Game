#include "LoadTests.h"
#include "elf.h"
#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Protocol/BlockIo.h>
#include  <Protocol/LoadedImage.h>
#include  <Protocol/SimpleFileSystem.h>
#include  <Library/DevicePathLib.h>
#include  <Guid/FileInfo.h>
#include  <Guid/Acpi.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Library/BaseMemoryLib.h>
#include "elf.h"
#include "relocate.h"
#include "memory_map.h"
#include "structs.h"

EFI_STATUS
        EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE *SystemTable
) {
  asm("cli");
  EFI_STATUS Status = EFI_SUCCESS;
  CHAR16 *kernel_path = L"\\kernel";
  EFI_PHYSICAL_ADDRESS parameters_address = 0x50000;
  EFI_PHYSICAL_ADDRESS entry_address;
  Status =  gBS->AllocatePages(AllocateAddress,EfiLoaderData,1,&parameters_address);
  Print(L"Allocate boot parameters status: %d\r\n", Status);
  struct boot *boot = (struct boot *) parameters_address;

  EFI_PHYSICAL_ADDRESS buffer_address;
  UINTN buffer_page_size;
  Status = LoadFile(kernel_path,&buffer_address,&buffer_page_size);
  Print(L"Kernel load status: %d\r\n", Status);
  elfhdr *elf = (elfhdr *) buffer_address;
  proghdr *ph = (proghdr *) ((uchar *) elf + elf->phoff);
  proghdr *eph = ph + elf->phnum;

  EFI_PHYSICAL_ADDRESS pages_start = ph->paddr;
  UINTN mask = ph.align - 1;
  EFI_PHYSICAL_ADDRESS pages_end = pages_start + ph->memsz;
  pages_end = (pages_end + mask) & ~mask;
  for (; ph < eph; ph++) {
    EFI_PHYSICAL_ADDRESS cur_start = ph->paddr;
    EFI_PHYSICAL_ADDRESS cur_end = start_address + ph->memsz;
    UINTN mask = ph.align - 1;
    cur_end = (cur_end + mask) & ~mask;
    if (cur_start < pages_start){
      pages_start = cur_start;
    }
    if (cur_end > pages_end) {
      pages_end = cur_end;
    }
  }
  UINTN page_size = ((pages_end - pages_start) / 4096) + 1;
  Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, page_size, &pages_start);
  Print(L"Allocate pages status: %d\r\n",Status);

  *ph = (proghdr *) ((uchar *) elf + elf->phoff);
  for (; ph < eph; ph++) {
    EFI_PHYSICAL_ADDRESS cur_start = ph->paddr;
    EFI_PHYSICAL_ADDRESS cur_end = start_address + ph->memsz;
    UINTN mask = ph->align - 1;
    cur_end = (cur_end + mask) & ~mask;
    CopyMem((VOID *)cur_start,(VOID *)(buffer_address + ph->offset), ph->filesz);
    if(ph->memsz > ph->filesz){
      SetMem((VOID *)(cur_start + ph->filesz), ph->memsz - ph->filesz, 0);
    }
  }

  *entry_address = (EFI_PHYSICAL_ADDRESS)elf->e_entry;
  FreePages((VOID *)buffer_address, buffer_page_size);

  boot->entry = (UINT64) entry_address;
  boot->entry_address = entry_address;
  boot->gdt_desc.size = sizeof(struct gdt)*3;
  boot->gdt_desc.gdt_addr = (UINT64)boot->gdt;
  boot->gdt_null.limit = 0;
  boot->gdt_null.base1 = 0;
  boot->gdt_null.base2 = 0;
  boot->gdt_null.access_byte = 0;
  boot->gdt_null.flags = 0;
  boot->gdt_null.base3 = 0;

  boot->gdt_code->limit = (0xffffffff >> 12) & 0xFFFF;
  boot->gdt_code->base1 = 0x0 & 0xFFFF;
  boot->gdt_code->base2 = (0x0 >> 16) & 0xFF;
  boot->gdt_code->access_byte = 0x90 | STA_X|STA_R;
  boot->gdt_code->flags = 0xC0 | ((0xffffffff >> 28) & 0xF);
  boot->gdt_code->base3 = (0x0 >> 24) & 0xFF;

  boot->gdt_data->limit = (0xffffffff >> 12) & 0xFFFF;
  boot->gdt_data->base1 = 0x0 & 0xFFFF;
  boot->gdt_data->base2 = (0x0 >> 16) & 0xFF;
  boot->gdt_data->access_byte = 0x90 | STA_W;
  boot->gdt_data->flags = 0xC0 | ((0xffffffff >> 28) & 0xF);
  boot->gdt_data->base3 = (0x0 >> 24) & 0xFF;

  struct MemoryMap MemoryMap = {4096,NULL,4096,0,0,0};
  Status = gBS->AllocatePool(EfiLoaderData, MemoryMap.BufferSize, &MemoryMap.Buffer);
  Print(L"Allocate memory map status: %d\r\n",Status);
  gBS->GetMemoryMap(
  &MemoryMap.MapSize,
  (EFI_MEMORY_DESCRIPTOR*)MemoryMap.Buffer,
  &MemoryMap.MapKey,
  &MemoryMap.DescriptorSize,
  &MemoryMap.DescriptorVersion);

  Status = gBS->ExitBootServices(ImageHandle, MemoryMap.MapKey);
  Print(L"Exit boot services status: %d\r\n",Status);
  asm("lgdt %0": : "m"(boot->gdt_desc));
  asm("push $0x08");
  asm("push %0": : "m"(boot->entry_addr));
  asm("lretq");
  while(1);
}
