# discussion

> `-bios` & `-pflash`

- `OvmfPkg/README`

* Use OVMF for QEMU firmware (3 options available)
  - Option 1: QEMU 1.6 or newer; Use QEMU -pflash parameter
    * QEMU/OVMF will use emulated flash, and **fully support** UEFI variables
    * Run qemu with: -pflash path/to/OVMF.fd
    * **Note that this option is required for running SecureBoot-enabled builds
      (-D SECURE_BOOT_ENABLE)**.
  - Option 2: Use QEMU -bios parameter
    * Note that UEFI variables will be **partially emulated**, and non-volatile
      variables may lose their contents after a reboot
    * Run qemu with: -bios path/to/OVMF.fd
  - Option 3: Use QEMU -L parameter
    * Note that UEFI variables will be **partially emulated**, and non-volatile
      variables may lose their contents after a reboot
    * Either copy, rename or symlink OVMF.fd => bios.bin
    * Use the QEMU -L parameter to specify the directory where the bios.bin
      file is located.

OVMF 给出的解释是:
- 安全启动需要
- 完全支持 UEFI variables

# links

- <https://superuser.com/questions/1703377/qemu-kvm-uefi-secure-boot-doesnt-work>

