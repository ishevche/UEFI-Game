#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/RngLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/DevicePathLib.h>
#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/Rng.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiFont.h>
#include <Protocol/HiiImage.h>
#include <Protocol/HiiImageDecoder.h>
#include <Protocol/LoadedImage.h>

typedef enum {
    EMPTY,
    WALL,
    START,
    END,
    UNKNOWN
} MAZE_CELL_TYPE;

typedef struct {
    INTN width;
    INTN height;
    MAZE_CELL_TYPE *mazeCells;
} MAZE;

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

EFI_STATUS GetRandom(OUT UINT8 *result) {
  EFI_STATUS Status = 0;
  *result = 10;
  EFI_RNG_PROTOCOL *rngProtocol;
  Status = gBS->LocateProtocol(&gEfiRngProtocolGuid, NULL, (VOID **) &rngProtocol);
  Print(L"Locate protocol: %d\n", Status);
  return Status;
}

void PrintGraphicsInfo(IN EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *graphicsInfo) {
  Print(L"Graphics info version: %d\n", graphicsInfo->Version);
  Print(L"Graphics info height: %d\n", graphicsInfo->VerticalResolution);
  Print(L"Graphics info width: %d\n", graphicsInfo->HorizontalResolution);
  Print(L"Graphics info scanline size: %d\n", graphicsInfo->PixelsPerScanLine);

  switch (graphicsInfo->PixelFormat) {
    case PixelRedGreenBlueReserved8BitPerColor:
      Print(L"Pixel format: PixelRedGreenBlueReserved8BitPerColor\n");
      break;
    case PixelBlueGreenRedReserved8BitPerColor:
      Print(L"Pixel format: PixelBlueGreenRedReserved8BitPerColor\n");
      break;
    case PixelBitMask:
      Print(L"Pixel format: PixelBitMask\n");
      break;
    case PixelBltOnly:
      Print(L"Pixel format: PixelBltOnly\n");
      break;
    case PixelFormatMax:
      Print(L"Pixel format: PixelFormatMax\n");
      break;
  }
  EFI_PIXEL_BITMASK pixelBitmask = graphicsInfo->PixelInformation;
  Print(L"Pixel bitmask red: %d\n", pixelBitmask.RedMask);
  Print(L"Pixel bitmask green: %d\n", pixelBitmask.GreenMask);
  Print(L"Pixel bitmask blue: %d\n", pixelBitmask.BlueMask);
  Print(L"Pixel bitmask reserved: %d\n", pixelBitmask.ReservedMask);
}

void ParseActor(IN CHAR16 *filename, IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *pixels, IN INTN box_side) {
  CHAR8 * spriteData;
  UINTN sizeOfSprite;
  ReadFile(filename, &spriteData, &sizeOfSprite);
  UINTN width = 0;
  while (spriteData[width] != '\n')
    width++;
  UINTN height = sizeOfSprite / (width + 1);
  for (INTN i = 0; i < width; ++i) {
    for (INTN j = 0; j < height; ++j) {
      switch (spriteData[j * (width + 1) + i]) {
        case 'k':
          pixels[j * box_side + i].Red = 0;
          pixels[j * box_side + i].Green = 0;
          pixels[j * box_side + i].Blue = 0;
          break;
        case 'b':
          pixels[j * box_side + i].Red = 47;
          pixels[j * box_side + i].Green = 54;
          pixels[j * box_side + i].Blue = 153;
          break;
        case 'l':
          pixels[j * box_side + i].Red = 77;
          pixels[j * box_side + i].Green = 109;
          pixels[j * box_side + i].Blue = 243;
          break;
        case 'y':
          pixels[j * box_side + i].Red = 255;
          pixels[j * box_side + i].Green = 242;
          pixels[j * box_side + i].Blue = 0;
          break;
        case 'g':
          pixels[j * box_side + i].Red = 255;
          pixels[j * box_side + i].Green = 194;
          pixels[j * box_side + i].Blue = 14;
          break;
        case 'r':
          pixels[j * box_side + i].Red = 180;
          pixels[j * box_side + i].Green = 180;
          pixels[j * box_side + i].Blue = 180;
          break;
        case 'w':
          pixels[j * box_side + i].Red = 255;
          pixels[j * box_side + i].Green = 255;
          pixels[j * box_side + i].Blue = 255;
          break;
        case 'p':
          pixels[j * box_side + i].Red = 255;
          pixels[j * box_side + i].Green = 205;
          pixels[j * box_side + i].Blue = 148;
          break;
        case 's':
          pixels[j * box_side + i].Red = 255;
          pixels[j * box_side + i].Green = 224;
          pixels[j * box_side + i].Blue = 189;
          break;
        case 'n':
          pixels[j * box_side + i].Red = 156;
          pixels[j * box_side + i].Green = 90;
          pixels[j * box_side + i].Blue = 60;
          break;
        case 'd':
          pixels[j * box_side + i].Red = 107;
          pixels[j * box_side + i].Green = 37;
          pixels[j * box_side + i].Blue = 7;
          break;
        case '.':
          break;
        default:
          return;
      }
    }
  }
}

//void Wordle() {
//  CHAR16 * word_list;
//  UINTN length;
//  Status = ReadFile((CHAR8 **)&word_list, &length);
//  Print(L"File size: %d\n", length);
//
//  UINT8 word_idx;
//  Status = GetRandom(&word_idx);
//
//  CHAR16 word[6] = {'\0', '\0', '\0', '\0', '\0', '\0'};
//  for (UINTN i = 0; i < 5; ++i) {
//    word[i] = word_list[6 * word_idx + i + 1];
//  }
//  Print(word);
//  Print(L"\n");
//  gBS->FreePool(word_list);
//}

//void graphicsInfo() {
//  Print(L"Graphics max mode: %d\n", graphicsProtocol->Mode->MaxMode);
//  Print(L"Graphics cur mode: %d\n", graphicsProtocol->Mode->Mode);
//  Print(L"Graphics size of info struct: %d\n", graphicsProtocol->Mode->SizeOfInfo);
//  Print(L"Graphics size of frame buffer: %d\n", graphicsProtocol->Mode->FrameBufferSize);
//  Print(L"Graphics frame buffer: %d\n", graphicsProtocol->Mode->FrameBufferBase);
//  Print(L"\n ----Info about all available modes:---- \n");
//  for (UINT32 idx = 0; idx < graphicsProtocol->Mode->MaxMode; ++idx) {
//    Print(L"\n\tMode #%d:\n", idx);
//    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION * info;
//    UINTN info_size;
//    graphicsProtocol->QueryMode(
//            graphicsProtocol, idx,
//            &info_size, &info
//    );
//    PrintGraphicsInfo(info);
//    gBS->FreePool(info);
//  }
//  PrintGraphicsInfo(graphicsProtocol->Mode->Info);
//}

void ParseMazeFile(IN CHAR8 *data, IN UINTN dataSize, OUT MAZE *maze) {
  maze->width = 0;
  while (data[maze->width] != '\n')
    maze->width++;
  maze->height = dataSize / (maze->width + 1);
  gBS->AllocatePool(EfiLoaderData, maze->width * maze->height * sizeof(MAZE_CELL_TYPE),
                    (VOID **) &maze->mazeCells);
  for (INTN i = 0; i < maze->height; ++i) {
    for (INTN j = 0; j < maze->width; ++j) {
      switch (data[i * (maze->width + 1) + j]) {
        case '#':
          maze->mazeCells[i * maze->width + j] = WALL;
          break;
        case '.':
          maze->mazeCells[i * maze->width + j] = EMPTY;
          break;
        case 'o':
          maze->mazeCells[i * maze->width + j] = START;
          break;
        case 'x':
          maze->mazeCells[i * maze->width + j] = END;
          break;
        default:
          maze->mazeCells[i * maze->width + j] = UNKNOWN;
          break;
      }
    }
  }
}

EFI_STATUS
EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE *SystemTable
) {
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *graphicsProtocol;
  EFI_HII_FONT_PROTOCOL *fontProtocol;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL * pixels;
  EFI_IMAGE_OUTPUT * screen_image;
  MAZE maze;
  CHAR8 * mazeData;
  UINTN mazeLen;
  UINT16 width;
  UINT16 height;


  Status = gBS->LocateProtocol(
          &gEfiGraphicsOutputProtocolGuid,
          NULL,
          (VOID **) &graphicsProtocol
  );
  Print(L"Locate graphics protocol: %d\n", Status);
  Status = gBS->LocateProtocol(
          &gEfiHiiFontProtocolGuid,
          NULL,
          (VOID **) &fontProtocol
  );
  Print(L"Locate font protocol: %d\n", Status);

  width = graphicsProtocol->Mode->Info->HorizontalResolution;
  height = graphicsProtocol->Mode->Info->VerticalResolution;

  Status = gBS->AllocatePool(EfiLoaderData, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * width * height,
                             (VOID **) &pixels);
  Print(L"Allocate: %d\n", Status);
  Status = gBS->AllocatePool(EfiLoaderData, sizeof(EFI_IMAGE_OUTPUT), (VOID **) &screen_image);
  Print(L"Allocate: %d\n", Status);

  screen_image->Height = height;
  screen_image->Width = width;
  screen_image->Image.Screen = graphicsProtocol;
  screen_image->Image.Bitmap = pixels;

  for (int i = 0; i < width * height; ++i) {
    pixels[i].Red = 0;
    pixels[i].Green = 0;
    pixels[i].Blue = 0;
  }

  ReadFile(L"labyrinth.txt", &mazeData, &mazeLen);
  ParseMazeFile(mazeData, mazeLen, &maze);

  INTN cell_width = width / maze.width;
  INTN cell_height = height / maze.height;
  INTN cell_side = cell_width < cell_height ? cell_width : cell_height;
  INTN height_margin = (height - cell_side * maze.height) / 2;
  INTN width_margin = (width - cell_side * maze.width) / 2;
  INTN offset = height_margin * width + width_margin;
  INTN posX = 0;
  INTN posY = 0;
  INTN endX = 0;
  INTN endY = 0;
  for (INTN i = 0; i < maze.height; ++i) {
    for (INTN j = 0; j < maze.width; ++j) {
      for (INTN a = 0; a < cell_side; ++a) {
        for (INTN b = 0; b < cell_side; ++b) {
          switch (maze.mazeCells[i * maze.width + j]) {
            case START:
              posX = j;
              posY = i;
            case EMPTY:
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Red = 0;
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Green = 0;
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Blue = 0;
              break;
            case WALL:
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Red = 112;
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Green = 157;
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Blue = 100;
              break;
            case END:
              endX = j;
              endY = i;
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Red = 0;
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Green = 0;
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Blue = 255;
              break;
            case UNKNOWN:
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Red = 255;
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Green = 0;
              pixels[offset + width * (cell_side * i + a) + cell_side * j + b].Blue = 0;
              break;
          }
        }
      }
    }
  }

  EFI_GRAPHICS_OUTPUT_BLT_PIXEL * emptyCell;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL * actor;
  gBS->AllocatePool(EfiLoaderData, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * cell_side * cell_side,
                    (VOID **) &emptyCell);
  gBS->AllocatePool(EfiLoaderData, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * cell_side * cell_side,
                    (VOID **) &actor);
  for (int i = 0; i < cell_side * cell_side; ++i) {
    emptyCell[i].Red = 0;
    emptyCell[i].Green = 0;
    emptyCell[i].Blue = 0;
    actor[i].Red = 0;
    actor[i].Green = 0;
    actor[i].Blue = 0;
  }
  ParseActor(L"sprite.txt", actor, cell_side);

  graphicsProtocol->Blt(
          graphicsProtocol,
          pixels,
          EfiBltBufferToVideo,
          0, 0,
          0, 0,
          width, height,
          width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
  );
  graphicsProtocol->Blt(
          graphicsProtocol,
          actor,
          EfiBltBufferToVideo,
          0, 0,
          posX * cell_side + width_margin,
          posY * cell_side + height_margin,
          cell_side, cell_side,
          cell_side * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
  );
  UINTN escaped = FALSE;
  INTN newX = posX;
  INTN newY = posY;
  while (!escaped) {
    UINTN what;
    Status = gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &what);
    EFI_INPUT_KEY Key;
    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    switch (Key.ScanCode) {
      case SCAN_UP:
        if (posY == 0) continue;
        if (maze.mazeCells[(posY - 1) * maze.width + posX] == WALL) continue;
        newY--;
        break;
      case SCAN_DOWN:
        if (posY == maze.height) continue;
        if (maze.mazeCells[(posY + 1) * maze.width + posX] == WALL) continue;
        newY++;
        break;
      case SCAN_LEFT:
        if (posX == 0) continue;
        if (maze.mazeCells[posY * maze.width + posX - 1] == WALL) continue;
        newX--;
        break;
      case SCAN_RIGHT:
        if (posX == maze.width) continue;
        if (maze.mazeCells[posY * maze.width + posX + 1] == WALL) continue;
        newX++;
        break;
      case SCAN_ESC:
        escaped = TRUE;
        break;
    }
    graphicsProtocol->Blt(
            graphicsProtocol,
            emptyCell,
            EfiBltBufferToVideo,
            0, 0,
            posX * cell_side + width_margin,
            posY * cell_side + height_margin,
            cell_side, cell_side,
            cell_side * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
    );
    graphicsProtocol->Blt(
            graphicsProtocol,
            actor,
            EfiBltBufferToVideo,
            0, 0,
            newX * cell_side + width_margin,
            newY * cell_side + height_margin,
            cell_side, cell_side,
            cell_side * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
    );
    posX = newX;
    posY = newY;
    if (newX == endX && newY == endY) {
      break;
    }
  }
  graphicsProtocol->
          Blt(
          graphicsProtocol,
          emptyCell,
          EfiBltVideoFill,
          0, 0,
          0, 0,
          width, height,
          width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
  );

  gBS->FreePool(pixels);
  gBS->FreePool(screen_image);
  gBS->FreePool(emptyCell);
  gBS->FreePool(actor);
  gBS->FreePool(mazeData);
  gBS->FreePool(maze.mazeCells);

  if (!escaped) {
    EFI_BOOT_MANAGER_LOAD_OPTION * BootOption;
    UINTN BootOptionCount;
    BootOption = EfiBootManagerGetLoadOptions(&BootOptionCount, LoadOptionTypeBoot);
    Print(L"Boot options: %d\n", BootOptionCount);
    for (UINTN i = 0; i < BootOptionCount; ++i) {
      Print(L"Boot entry #%d: %s\n", i, BootOption[i].Description);
    }
    EfiBootManagerBoot(&BootOption[3]);
  }

  return Status;
}
