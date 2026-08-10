.extern __efi64

.section .boot.text

.globl _efi_start
_efi_start:
    call __efi64
