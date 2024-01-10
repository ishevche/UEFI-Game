#ifndef OSPROJECT_BMPREADER_H
#define OSPROJECT_BMPREADER_H

#include "Common.h"

#pragma pack(1)
typedef struct CIEXYZ {
    INT32 ciexyzX;
    INT32 ciexyzY;
    INT32 ciexyzZ;
} CIEXYZ;

typedef struct tagICEXYZTRIPLE {
    CIEXYZ ciexyzRed;
    CIEXYZ ciexyzGreen;
    CIEXYZ ciexyzBlue;
} CIEXYZTRIPLE;

typedef struct BMP_HEADER {
    UINT16 Signature;
    UINT32 FileSize;
    UINT32 Unused;
    UINT32 PixelsOffset;
    UINT32 HeaderSize;
    INT32 Width;
    INT32 Height;
    UINT16 Planes;
    UINT16 BitsPerPixel;
    UINT32 CompressionMethod;
    UINT32 ImageSize;
    INT32 XPXLsPerMeter;
    INT32 YPXLsPerMeter;
    UINT32 ColorsUsed;
    UINT32 ColorsImportant;
    UINT32 RedMask;
    UINT32 GreenMask;
    UINT32 BlueMask;
    UINT32 AlphaMask;
    UINT32 CSType;
    CIEXYZTRIPLE Endpoints;
    UINT32 GammaRed;
    UINT32 GammaGreen;
    UINT32 GammaBlue;
    UINT32 Intent;
    UINT32 ProfileData;
    UINT32 ProfileSize;
    UINT32 Reserved;
} BMP_HEADER;

typedef struct BMP_PIXEL {
    UINT8 Blue;
    UINT8 Green;
    UINT8 Red;
    UINT8 Alpha;
} BMP_PIXEL;
#pragma pack(0)

EFI_STATUS ReadBMP(
        IN EFI_HANDLE FileSystem,
        IN CHAR16 *Filename,
        OUT INT32 *Width,
        OUT INT32 *Height,
        OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL **Pixels
);

#endif //OSPROJECT_BMPREADER_H
