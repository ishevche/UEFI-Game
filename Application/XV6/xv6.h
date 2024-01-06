#ifndef EDK2_STABLE202308_LOAD_TESTS_H
#define EDK2_STABLE202308_LOAD_TESTS_H

struct __attribute__((packed)) boot {
    UINT64 entry;
    UINT64 entry_addr;
    UINT64 madt_addr;
    struct gdt gdt_null;
    struct gdt gdt_code;
    struct gdt gdt_data;
    struct gdt_desc gdt_desc;
};

struct __attribute__((packed)) gdt {
    UINT16 limit;
    UINT16 base1;
    UINT8 base2;
    UINT8 access_byte;
    UINT8 flags;
    UINT8 base3;
};

struct __attribute__((packed)) gdt_desc {
    UINT16 size;
    UINT64 gdt_addr;
};

#endif
