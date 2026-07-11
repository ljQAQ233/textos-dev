# x86_64
{
  pkgs ? import <nixpkgs> { },
}:

let
  logo = ''
     _____         _    ___  ____  
    |_   _|____  _| |_ / _ \/ ___| 
      | |/ _ \ \/ / __| | | \___ \ 
      | |  __/>  <| |_| |_| |___) |
      |_|\___/_/\_\\__|\___/|____/
  '';
in
pkgs.mkShellNoCC {
  nativeBuildInputs = with pkgs; [
    stdenv
    gnumake
    patchelf
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
    awk '{ print "\033[32m" $0 "\033[0m" }' <<< "${logo}"
    echo Development environment is ready!
  '';

  CROSS_COMPILE = "";
  QEMU_HOME = "${pkgs.qemu_full}";
}
