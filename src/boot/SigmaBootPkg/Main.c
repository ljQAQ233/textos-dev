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

EFI_STATUS
GetMemoryMap (
  MAP_INFO  *Info
  )
{
  ASSERT (Info != NULL);

  EFI_STATUS  Status  = EFI_SUCCESS;
  VOID        *Descs  = NULL;
  UINTN       MapSiz  = 0;
  UINTN       PageNum = 0;

Retry:
  Status = gBS->GetMemoryMap (
                  &MapSiz,
                  Descs,
                  &Info->MapKey,
                  &Info->DescSize,
                  &Info->DescVersion
                  );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    if (Descs != NULL) {
      gBS->FreePages ((EFI_PHYSICAL_ADDRESS)Descs, PageNum);
    }

    PageNum = EFI_SIZE_TO_PAGES (MapSiz);
    Status  = gBS->AllocatePages (
                     AllocateAnyPages,
                     EfiReservedMemoryType,
                     PageNum,
                     (EFI_PHYSICAL_ADDRESS *)&Descs
                     );
    ERR_RETS (Status);

    goto Retry;
  }

  ERR_RETS (Status);

  Info->Descs    = Descs;
  Info->MapSize  = MapSiz;
  Info->MapCount = Info->MapSize / Info->DescSize;
  return Status;
}

EFI_STATUS
ExitBootServices (
  IN EFI_HANDLE  ImageHandle,
  OUT MAP_INFO   *Info
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  Status = GetMemoryMap (Info);
  ERR_RETS (Status);

  Status = gBS->ExitBootServices (ImageHandle, Info->MapKey);
  ERR_RETS (Status);

  return Status;
}

BOOT_CONFIG  Config;

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

  EFI_PHYSICAL_ADDRESS  KernelEntry;
  EFI_PHYSICAL_ADDRESS  KernelBase;
  UINT64                KernelSize;

  KernelLoad (KernelPath, &KernelEntry, &KernelBase, &KernelSize);

  EfiGetSystemConfigurationTable (&gEfiAcpi20TableGuid, &Config.AcpiTable);

  MAP_INFO  *MapInfo = AllocateRuntimePages (EFI_SIZE_TO_PAGES (sizeof (MAP_INFO)));

  // ExitBootServices (ImageHandle, MapInfo);
  Config.Magic                 = SIGNATURE_64 ('T', 'E', 'X', 'T', 'O', 'S', 'B', 'T');
  Config.Video.FrameBufferBase =
    gGraphicsOutputProtocol->Mode->FrameBufferBase;
  Config.Video.FrameBufferSize =
    gGraphicsOutputProtocol->Mode->FrameBufferSize;
  Config.Video.HorizontalResolution =
    gGraphicsOutputProtocol->Mode->Info->HorizontalResolution;
  Config.Video.VerticalResolution =
    gGraphicsOutputProtocol->Mode->Info->VerticalResolution;

  Config.Memory.MapInfo  = MapInfo;
  Config.RuntimeServices = SystemTable->RuntimeServices;

  ((VOID (*)(long, long)) KernelEntry)(Config.Magic, (long)&Config);

  return EFI_SUCCESS;
}
