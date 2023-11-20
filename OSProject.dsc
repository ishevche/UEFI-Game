[Defines]
  PLATFORM_NAME           = OSProject
  PLATFORM_GUID           = 0f2d1311-600f-4aba-9867-293e8a3368a2
  PLATFORM_VERSION        = 0.1
  DSC_SPECIFICATION       = 0x00010005
  OUTPUT_DIRECTORY        = Build/OSProject
  SUPPORTED_ARCHITECTURES = X64
  BUILD_TARGETS           = DEBUG
  SKUID_IDENTIFIER        = DEFAULT

!include MdePkg/MdeLibs.dsc.inc

[LibraryClasses]
  UefiApplicationEntryPoint | MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiLib | MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib | MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib | MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib | MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  BaseMemoryLib | MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  UefiBootServicesTableLib | MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  DevicePathLib | MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiRuntimeServicesTableLib | MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  BaseLib | MdePkg/Library/BaseLib/BaseLib.inf
  DebugLib | MdePkg/Library/UefiDebugLibStdErr/UefiDebugLibStdErr.inf
  DebugPrintErrorLevelLib | MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  RngLib | MdePkg/Library/BaseRngLib/BaseRngLib.inf

[Components]
  Wordle/Application/Wordle/Wordle.inf