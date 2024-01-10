#ifndef OSPROJECT_FILESYSTEMS_H
#define OSPROJECT_FILESYSTEMS_H

#include "Common.h"

typedef struct partition {
    EFI_HANDLE handle;
    GUID *guid;
} PARTITION;

extern EFI_HANDLE SelfFSHandle;

EFI_STATUS ReadFile(IN EFI_HANDLE PartitionHandle,
                    IN CHAR16 *Filename,
                    OUT VOID **Data,
                    OUT UINTN *Size);

EFI_STATUS LocateCurFSHandle();

EFI_STATUS LocateFSPartitions(OUT PARTITION **Partitions, OUT UINTN *PartitionsCount);

#endif //OSPROJECT_FILESYSTEMS_H
