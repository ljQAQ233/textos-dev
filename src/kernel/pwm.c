// power manager

#include <io.h>

#define try(x) { do { x; } while(true); }

// start qemu with cmdline -device isa-debug-exit
static void qemu_shutdown()
{
    try(outw(0x604, 0x2000));
}

#include <boot.h>
#include <textos/uefi.h>

// TODO: test it
static void uefi_shutdown()
{
    EFI_RUNTIME_SERVICES *rt = bconfig_get()->runtime;
    try(rt->ResetSystem(
            EfiResetShutdown,
            EFI_SUCCESS,
            0, NULL
        ));
}

#include <textos/syscall.h>

__SYSCALL_DEFINE0(int, poweroff)
{
    // TODO: handle tasks

    // qemu_shutdown();
    uefi_shutdown();

    return -1;
}

