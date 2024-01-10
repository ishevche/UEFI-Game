#ifndef EDK2_STABLE202308_LOAD_TESTS_H
#define EDK2_STABLE202308_LOAD_TESTS_H

#define STA_X     0x8       // Executable segment
#define STA_E     0x4       // Expand down (non-executable segments)
#define STA_C     0x4       // Conforming code segment (executable only)
#define STA_W     0x2       // Writeable (non-executable segments)
#define STA_R     0x2       // Readable (executable segments)
#define STA_A     0x1       // Accessed

typedef struct __attribute__((packed)) gdt {
    UINT16 limit;
    UINT16 base1;
    UINT8 base2;
    UINT8 access_byte;
    UINT8 flags;
    UINT8 base3;
} gdt;

typedef struct __attribute__((packed)) gdt_desc {
    UINT16 size;
    UINT64 gdt_addr;
} gdt_desc;

typedef struct __attribute__((packed)) boot {
    UINT64 entry;
    UINT64 entry_addr;
    UINT64 madt_addr;
    gdt gdt[3];
    gdt_desc *gdt_desc;
} boot;

typedef struct MemoryMap {
    VOID *Buffer;
    UINTN MapSize;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
} MemoryMap;

#endif
