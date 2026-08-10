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
  mapinfo_t  *Info
  )
{
  ASSERT (Info != NULL);

  EFI_STATUS  Status  = EFI_SUCCESS;
  VOID        *Descs  = NULL;
  UINTN       MapSiz  = 0;
  UINTN       PageNum = 0;
  UINTN       MapKey;
  UINTN       DescriptorSize;
  UINT32      DescriptorVersion;

Retry:
  Status = gBS->GetMemoryMap (
                  &MapSiz,
                  Descs,
                  &MapKey,
                  &DescriptorSize,
                  &DescriptorVersion
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

  Info->map    = Descs;
  Info->mapsiz   = MapSiz;
  Info->desccnt = Info->mapsiz / DescriptorSize;
  Info->mapkey   = MapKey;
  Info->descsiz  = DescriptorSize;
  Info->descver  = DescriptorVersion;
  return Status;
}

EFI_STATUS
ExitBootServices (
  IN EFI_HANDLE  ImageHandle,
  OUT mapinfo_t  *Info
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  Status = GetMemoryMap (Info);
  ERR_RETS (Status);

  Status = gBS->ExitBootServices (ImageHandle, (UINTN)Info->mapkey);
  ERR_RETS (Status);

  return Status;
}

bconfig_t  Config;

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

  EfiGetSystemConfigurationTable (&gEfiAcpi20TableGuid, &Config.acpi);

  mapinfo_t  *MapInfo = AllocatePool (sizeof (mapinfo_t));

  // ExitBootServices (ImageHandle, MapInfo);
  Config.magic   = TEXTOS_BOOT_MAGIC;
  Config.fb      = gGraphicsOutputProtocol->Mode->FrameBufferBase;
  Config.fb_siz  = gGraphicsOutputProtocol->Mode->FrameBufferSize;
  Config.hor     = gGraphicsOutputProtocol->Mode->Info->HorizontalResolution;
  Config.ver     = gGraphicsOutputProtocol->Mode->Info->VerticalResolution;
  Config.mapinfo     = MapInfo;
  Config.runtime = SystemTable->RuntimeServices;

  ((VOID (*)(long, long)) KernelEntry)(Config.magic, (long)&Config);

  return EFI_SUCCESS;
}
