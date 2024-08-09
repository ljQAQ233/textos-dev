#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Guid/Acpi.h>
#include <IndustryStandard/Acpi.h>

#include <Boot.h>
#include <Logo.h>
#include <Config.h>
#include <Graphics.h>
#include <Font.h>
#include <File.h>
#include <Memory.h>
#include <Kernel.h>

typedef struct {
  UINT64 Hor;
  UINT64 Ver;
  UINT64 FrameBuffer;
  UINT64 FrameBufferSize;
} GRAPHICS_CONFIG;

/*
   describes where the memory is allocated with the type EfiReservedMemoryType
   which would be used by kernel, kernel also depends on them before completing
   memory initialization.
*/
typedef struct {
  UINT8   Valid;
  UINT64  PageNum;
  VOID    *Pointer;
} ALLOCATE_INFO;

typedef struct {
  ALLOCATE_INFO Allocate[16];
  VOID          *Map;
  VOID          *KernalPages;
} MEMORY_CONFIG;

typedef struct {
  UINT64          Magic;
  GRAPHICS_CONFIG Graphics;
  MEMORY_CONFIG   Memory;
  VOID            *AcpiTab;
  VOID            *Runtime;
} BOOT_CONFIG;

EFI_STATUS ExitBootServices (
        IN     EFI_HANDLE ImageHandle,
           OUT MAP_INFO   *Info
    )
{
    EFI_STATUS Status = EFI_SUCCESS;

    Status = MemoryGetMap (Info);
    ERR_RETS(Status);

    Status = gBS->ExitBootServices (
            ImageHandle,
            Info->MapKey
        );
    ERR_RETS(Status);

    return Status;
}

BOOT_CONFIG Config;

VOID RegisterMemory (
        UINT64 PageNum,
        VOID   *Pointer
        )
{
    STATIC UINTN MmIdx = 0;

    Config.Memory.Allocate[MmIdx].PageNum = PageNum;
    Config.Memory.Allocate[MmIdx].Pointer = Pointer;
    Config.Memory.Allocate[MmIdx].Valid = TRUE;

    MmIdx++;
}

EFI_STATUS EFIAPI UefiMain (
        IN EFI_HANDLE        ImageHandle,
        IN EFI_SYSTEM_TABLE  *SystemTable
        )
{
    Breakpoint();

    InitializeGraphicsServices();
    InitializeFileServices();

    InitializeConfig();

    CHAR16 *KernelPath = ConfigGetStringChar16 ("kernel", D_KERNEL_PATH);

    KERNEL_PAGE *KernelPages;
    EFI_PHYSICAL_ADDRESS KernelEntry;

    KernelLoad (KernelPath, &KernelEntry, &KernelPages);
    CHAR16 *FontPath = ConfigGetStringChar16 ("Font", D_FONT_PATH);
    FONT_CONFIG *Font = AllocateZeroPool (sizeof (FONT_CONFIG));
    FontLoad (FontPath,Font);

    EfiGetSystemConfigurationTable (&gEfiAcpi20TableGuid, &Config.AcpiTab);

    UINT64 PML4Addr;
    InitializePageTab (KernelPages, &PML4Addr);
    UpdateCr3 (PML4Addr,0);

    MAP_INFO *Map = AllocateRuntimePages(EFI_SIZE_TO_PAGES(sizeof(MAP_INFO)));
    RegisterMemory(EFI_SIZE_TO_PAGES(sizeof(MAP_INFO)), Map);
    ExitBootServices(ImageHandle, Map);

    Config.Magic = SIGNATURE_64('T', 'E', 'X', 'T', 'O', 'S', 'B', 'T');
    Config.Graphics.FrameBuffer     = gGraphicsOutputProtocol->Mode->FrameBufferBase;
    Config.Graphics.FrameBufferSize = gGraphicsOutputProtocol->Mode->FrameBufferSize;
    Config.Graphics.Hor             = gGraphicsOutputProtocol->Mode->Info->HorizontalResolution;
    Config.Graphics.Ver             = gGraphicsOutputProtocol->Mode->Info->VerticalResolution;

    Config.Memory.Map = Map;
    Config.Memory.KernalPages = KernelPages;

    Config.Runtime = SystemTable->RuntimeServices;

    ((VOID (*)(BOOT_CONFIG *))KernelEntry)(&Config);

    return EFI_SUCCESS;
}
