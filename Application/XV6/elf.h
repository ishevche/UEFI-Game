// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
typedef struct elfhdr {
    UINT32 magic;  // must equal ELF_MAGIC
    UINT8 elf[12];
    UINT16  type;
    UINT16  machine;
    UINT32 version;
    UINT32 entry;
    UINT32 phoff;
    UINT32 shoff;
    UINT32 flags;
    UINT16 ehsize;
    UINT16 phentsize;
    UINT16 phnum;
    UINT16 shentsize;
    UINT16 shnum;
    UINT16 shstrndx;
} elfhdr;

// Program section header
typedef struct proghdr {
    UINT32 type;
    UINT32 off;
    UINT32 vaddr;
    UINT32 paddr;
    UINT32 filesz;
    UINT32 memsz;
    UINT32 flags;
    UINT32 align;
} proghdr;

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4
