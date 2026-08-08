#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Uefi.h>

#include <Guid/Acpi.h>
#include <IndustryStandard/Acpi.h>

#include <Boot.h>
#include <Config.h>
#include <File.h>
#include <Font.h>
#include <Graphics.h>
#include <Kernel.h>
#include <Logo.h>
#include <Memory.h>

EFI_STATUS
ExitBootServices (
  IN EFI_HANDLE  ImageHandle,
  OUT MAP_INFO   *Info
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  Status = MemoryGetMap (Info);
  ERR_RETS (Status);

  Status = gBS->ExitBootServices (ImageHandle, Info->MapKey);
  ERR_RETS (Status);

  return Status;
}

BOOT_CONFIG  Config;

VOID
RegisterMemory (
  UINT64  PageNum,
  VOID    *Pointer
  )
{
  STATIC UINTN  MmIdx = 0;

  Config.Memory.Allocation[MmIdx].PageNum = PageNum;
  Config.Memory.Allocation[MmIdx].Pointer = Pointer;
  Config.Memory.Allocation[MmIdx].IsValid = TRUE;

  MmIdx++;
}

EFI_STATUS EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Breakpoint ();

  InitializeGraphicsServices ();
  InitializeFileServices ();

  InitializeConfig ();

  UINT64  HorConfig = ConfigGetNumUint64 ("hor", D_HOR);
  UINT64  VerConfig = ConfigGetNumUint64 ("ver", D_VER);

  GraphicsResolutionSet (HorConfig, VerConfig);

  CHAR16  *KernelPath = ConfigGetStringChar16 ("kernel", D_KERNEL_PATH);

  KERNEL_PAGE           *KernelPages;
  EFI_PHYSICAL_ADDRESS  KernelEntry;

  KernelLoad (KernelPath, &KernelEntry, &KernelPages);

  EfiGetSystemConfigurationTable (&gEfiAcpi20TableGuid, &Config.AcpiTable);

  UINT64  PML4Address;

  InitializePageTable (KernelPages, &PML4Address);
  UpdateCr3 (PML4Address, 0);

  MAP_INFO  *MapInfo = AllocateRuntimePages (EFI_SIZE_TO_PAGES (sizeof (MAP_INFO)));

  RegisterMemory (EFI_SIZE_TO_PAGES (sizeof (MAP_INFO)), MapInfo);
  ExitBootServices (ImageHandle, MapInfo);
  Config.Magic                 = SIGNATURE_64 ('T', 'E', 'X', 'T', 'O', 'S', 'B', 'T');
  Config.Video.FrameBufferBase =
    gGraphicsOutputProtocol->Mode->FrameBufferBase;
  Config.Video.FrameBufferSize =
    gGraphicsOutputProtocol->Mode->FrameBufferSize;
  Config.Video.HorizontalResolution =
    gGraphicsOutputProtocol->Mode->Info->HorizontalResolution;
  Config.Video.VerticalResolution =
    gGraphicsOutputProtocol->Mode->Info->VerticalResolution;

  Config.Memory.MapInfo     = MapInfo;
  Config.Memory.KernalPages = KernelPages;
  Config.RuntimeServices    = SystemTable->RuntimeServices;

  ((VOID (*)(long, long)) KernelEntry)(Config.Magic, (long)&Config);

  return EFI_SUCCESS;
}
