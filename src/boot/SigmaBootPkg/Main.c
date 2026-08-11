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

  Info->map     = Descs;
  Info->mapsiz  = MapSiz;
  Info->desccnt = Info->mapsiz / DescriptorSize;
  Info->mapkey  = MapKey;
  Info->descsiz = DescriptorSize;
  Info->descver = DescriptorVersion;
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

EFI_STATUS
ReadKey (
  OUT EFI_INPUT_KEY  *Key
  )
{
  EFI_STATUS  Status;
  UINTN       Index;

  gST->ConIn->Reset (gST->ConIn, FALSE);

  Status = gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return gST->ConIn->ReadKeyStroke (gST->ConIn, Key);
}

bconfig_t  Config;

EFI_STATUS
Boot (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS        Status      = EFI_SUCCESS;
  UINT64            HorConfig   = ConfigGetNumUint64 ("hor", D_HOR);
  UINT64            VerConfig   = ConfigGetNumUint64 ("ver", D_VER);
  CHAR16            *KernelPath = ConfigGetStringChar16 ("kernel", D_KERNEL_PATH);
  PHYSICAL_ADDRESS  KernelEntry;
  PHYSICAL_ADDRESS  KernelBase;
  UINT64            KernelSize;
  mapinfo_t         *MapInfo = AllocatePool (sizeof (mapinfo_t));

  Status = GraphicsResolutionSet (HorConfig, VerConfig);

  Status = EfiGetSystemConfigurationTable (&gEfiAcpi20TableGuid, &Config.acpi);
  if (EFI_ERROR (Status)) {
    Print (L"Failed to get ACPI table\n");
    goto Error;
  }

  Status = KernelLoad (KernelPath, &KernelEntry, &KernelBase, &KernelSize);
  if (EFI_ERROR (Status)) {
    Print (L"Failed to load kernel\n");
    goto Error;
  }

  Status = ExitBootServices (ImageHandle, MapInfo);
  if (EFI_ERROR (Status)) {
    Print (L"Failed to exit boot services\n");
    goto Error;
  }

  Config.magic     = TEXTOS_BOOT_MAGIC;
  Config.fb        = gGraphicsOutputProtocol->Mode->FrameBufferBase;
  Config.fb_siz    = gGraphicsOutputProtocol->Mode->FrameBufferSize;
  Config.hor       = gGraphicsOutputProtocol->Mode->Info->HorizontalResolution;
  Config.ver       = gGraphicsOutputProtocol->Mode->Info->VerticalResolution;
  Config.mapinfo   = MapInfo;
  Config.runtime   = SystemTable->RuntimeServices;
  Config.load_base = KernelBase;
  Config.load_size = KernelSize;
  Config.phy_entry = KernelEntry;

  ((VOID (*)(long, long)) KernelEntry)(Config.magic, (long)&Config);

Error:
  return Status;
}

EFI_STATUS EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;

  Breakpoint ();

  InitializeGraphicsServices ();
  InitializeFileServices ();
  InitializeConfig ();

  for ( ; ;) {
    Status = Boot (ImageHandle, SystemTable);
    if (EFI_ERROR (Status)) {
      Print (L"Boot failed. Last status = %r\n", Status);
      Print (L"  Press 'r' to retry\n");
      Print (L"  Press other keys to exit\n");
      if (EFI_ERROR (ReadKey (&Key)) || (Key.UnicodeChar != L'r')) {
        break;
      }
    }
  }

  return Status;
}
