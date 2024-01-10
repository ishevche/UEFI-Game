#include "BMPReader.h"
#include "FileSystems.h"

EFI_STATUS ReadBMP(
        IN EFI_HANDLE FileSystem,
        IN CHAR16 *Filename,
        OUT INT32 *Width,
        OUT INT32 *Height,
        OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL **Pixels
) {
  if (Width == NULL || Height == NULL || Filename == NULL || Pixels == NULL)
    return EFI_INVALID_PARAMETER;
  UINTN FileSize;
  CHAR8 * FileData;
  EFI_STATUS Status = ReadFile(FileSystem, Filename, (VOID **) &FileData, &FileSize);
  if (CheckError(Status, L"Reading the BMP file")) return Status;
  BMP_HEADER *Header = (BMP_HEADER *) FileData;
  if (Header->Signature != 0x4d42 ||
      Header->HeaderSize != 0x7c ||
      Header->BitsPerPixel != 0x20 ||
      Header->CompressionMethod != 3 ||
      Header->AlphaMask != 0xff000000 ||
      Header->RedMask != 0x00ff0000 ||
      Header->GreenMask != 0x0000ff00 ||
      Header->BlueMask != 0x000000ff) {
//    printf("Signature        : %d\n", Header->Signature != 0x4d42);
//    printf("HeaderSize       : %d\n", Header->HeaderSize != 0x7c);
//    printf("BitsPerPixel     : %d\n", Header->BitsPerPixel != 0x20);
//    printf("CompressionMethod: %d\n", Header->CompressionMethod != 3);
//    printf("AlphaMask        : %d\n", Header->AlphaMask != 0xff000000);
//    printf("RedMask          : %d\n", Header->RedMask != 0x00ff0000);
//    printf("GreenMask        : %d\n", Header->GreenMask != 0x0000ff00);
//    printf("BlueMask         : %d\n", Header->BlueMask != 0x000000ff);
    CheckError(EFI_UNSUPPORTED, CatSPrint((CHAR16 *) L"while reading reading texture %s", Filename));
    return EFI_UNSUPPORTED;
  }
  *Width = Header->Width;
  *Height = Header->Height;
  *Pixels = AllocateZeroPool((*Width) * (*Height) * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

  CHAR8 * CurPixelStart = FileData + Header->PixelsOffset;
  for (INT32 y = *Height - 1; y >= 0; --y) {
    for (INT32 x = 0; x < *Width; ++x) {
      BMP_PIXEL *CurFilePixel = (BMP_PIXEL *) CurPixelStart;
      EFI_GRAPHICS_OUTPUT_BLT_PIXEL * CurStoragePixel = (*Pixels) + y * (*Width) + x;
      CurStoragePixel->Red = CurFilePixel->Red;
      CurStoragePixel->Green = CurFilePixel->Green;
      CurStoragePixel->Blue = CurFilePixel->Blue;
      CurStoragePixel->Reserved = CurFilePixel->Alpha;
      CurPixelStart += 4;
    }
  }

  MyFreePool(FileData);
  return EFI_SUCCESS;
}
