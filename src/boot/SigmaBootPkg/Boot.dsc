[Defines]
    PLATFORM_NAME           = SigmaBootPkg
    PLATFORM_GUID           = 20c0491a-6a8b-4837-b30d-6a29ab35c14b
    !ifndef $(VERSION)
     DEFINE VERSION         = 20232422
    !endif
    PLATFORM_VERSION        = $(VERSION)
    DSC_SPECIFICATION       = 0x00010005
    SUPPORTED_ARCHITECTURES = X64 | IA32 | AARCH64 | ARM
    BUILD_TARGETS           = DEBUG|RELEASE
    !ifndef $(OUTPUT)
     DEFINE OUTPUT          = ./Build
    !endif
    OUTPUT_DIRECTORY        = $(OUTPUT)

[LibraryClasses]
    BaseLib                      | MdePkg/Library/BaseLib/BaseLib.inf
    UefiLib                      | MdePkg/Library/UefiLib/UefiLib.inf
    UefiApplicationEntryPoint    | MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
    PrintLib                     | MdePkg/Library/BasePrintLib/BasePrintLib.inf
    PcdLib                       | MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    DevicePathLib                | MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
    BaseMemoryLib                | MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
    MemoryAllocationLib          | MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
    UefiBootServicesTableLib     | MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
    UefiRuntimeServicesTableLib  | MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
    !if $(DBG_PRINT)
     DebugLib                    | MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
     DebugPrintErrorLevelLib     | MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
    !else
     DebugLib                    | MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
    !endif

[PcdsFixedAtBuild]
    !if $(DBG_PRINT)
     gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask       |0x13
     gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel    |0x80000040
    !endif

[Components]
    SigmaBootPkg/Boot.inf

[BuildOptions]
    *_*_*_CC_FLAGS = $(CFLAGS)
