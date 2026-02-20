# x86_64
{
  pkgs ? import <nixpkgs> { },
}:

pkgs.mkShellNoCC {
  nativeBuildInputs = with pkgs; [
    stdenv
    gnumake
    gcc-unwrapped
    binutils-unwrapped
    gdb
    bear
    # edk2 / ovmf
    nasm
    perl
    libuuid
    util-linux
    acpica-tools
    python3Packages.python

    # emu
    iptables
    iproute2
    qemu_full
  ];

  shellHook = ''
    awk '{ print "\033[32m" $0 "\033[0m" }' < init/logo.txt
    echo Development environment is ready!
  '';

  CROSS_COMPILE = "";
  QEMU_HOME = "${pkgs.qemu_full}";
}
