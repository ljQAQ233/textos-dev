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

typedef struct {
  VOID   *Map;
  VOID   *KernalPages;
} MEMORY_CONFIG;

typedef struct {
  UINT64          Magic;
  GRAPHICS_CONFIG Graphics;
  MEMORY_CONFIG   Memory;
  VOID            *AcpiTab;
} BOOT_CONFIG;

/* From tanyugang's Code,and I modified it,very thanks! */

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

    Info->MapCount = Info->MapSiz / Info->DescSiz;

    return Status;
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

    BOOT_CONFIG *Config = AllocateZeroPool (sizeof (BOOT_CONFIG));

    CHAR16 *KernelPath = ConfigGetStringChar16 ("kernel", D_KERNEL_PATH);

    KERNEL_PAGE *KernelPages;
    EFI_PHYSICAL_ADDRESS KernelEntry;

    KernelLoad (KernelPath, &KernelEntry, &KernelPages);
    CHAR16 *FontPath = ConfigGetStringChar16 ("Font", D_FONT_PATH);
    FONT_CONFIG *Font = AllocateZeroPool (sizeof (FONT_CONFIG));
    FontLoad (FontPath,Font);

    EfiGetSystemConfigurationTable (&gEfiAcpi20TableGuid, &Config->AcpiTab);

    MAP_INFO *Map = AllocateZeroPool (sizeof (MAP_INFO));

    UINT64 PML4Addr;
    InitializePageTab (KernelPages, &PML4Addr);
    UpdateCr3 (PML4Addr,0);

    ExitBootServices (ImageHandle, Map);

    Config->Magic = SIGNATURE_64('T', 'E', 'X', 'T', 'O', 'S', 'B', 'T');
    Config->Graphics.FrameBuffer     = gGraphicsOutputProtocol->Mode->FrameBufferBase;
    Config->Graphics.FrameBufferSize = gGraphicsOutputProtocol->Mode->FrameBufferSize;
    Config->Graphics.Hor             = gGraphicsOutputProtocol->Mode->Info->HorizontalResolution;
    Config->Graphics.Ver             = gGraphicsOutputProtocol->Mode->Info->VerticalResolution;

    Config->Memory.Map = Map;
    Config->Memory.KernalPages = KernelPages;

    ((VOID (*)(BOOT_CONFIG *))KernelEntry)(Config);

    return EFI_SUCCESS;
}
