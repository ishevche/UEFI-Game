#ifndef OSPROJECT_GPT_FS_H
#define OSPROJECT_GPT_FS_H

#pragma pack(1)
typedef struct {
    UINT8   flags;
    UINT8   start_chs[3];
    UINT8   type;
    UINT8   end_chs[3];
    UINT32  start_lba;
    UINT32  size;
} MBR_PART_INFO;


typedef struct {
    UINT8            code[440];
    UINT32           diskSignature;
    UINT16           nulls;
    MBR_PART_INFO    partitions[4];
    UINT16           MBRSignature;
} MBR_RECORD;

typedef struct {
    UINT64  signature;
    UINT32  spec_revision;
    UINT32  header_size;
    UINT32  header_crc32;
    UINT32  reserved;
    UINT64  header_lba;
    UINT64  alternate_header_lba;
    UINT64  first_usable_lba;
    UINT64  last_usable_lba;
    UINT8   disk_guid[16];
    UINT64  entry_lba;
    UINT32  entry_count;
    UINT32  entry_size;
    UINT32  entry_crc32;
    UINT8   reserved2[420];
} GPT_HEADER;

typedef struct {
    UINT8   type_guid[16];
    UINT8   partition_guid[16];
    UINT64  start_lba;
    UINT64  end_lba;
    UINT64  attributes;
    CHAR16  name[36];
} GPT_ENTRY;

typedef struct _gpt_data {
    MBR_RECORD         *ProtectiveMBR;
    GPT_HEADER         *Header;
    GPT_ENTRY          *Entries;
    struct _gpt_data   *NextEntry;
} GPT_DATA;
#pragma pack(0)

#endif //OSPROJECT_GPT_FS_H
