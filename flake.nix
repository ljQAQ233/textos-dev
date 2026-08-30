{
  description = "textOS development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs =
    {
      self,
      nixpkgs,
    }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      devShells.${system}.default = pkgs.mkShellNoCC {
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

        CROSS_COMPILE = "";
        QEMU_HOME = "${pkgs.qemu_full}";
      };
    };
}
