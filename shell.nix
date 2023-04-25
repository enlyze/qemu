{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = [
    pkgs.python3
    pkgs.ninja
    pkgs.pkg-config
    pkgs.bison
    pkgs.flex
    pkgs.gtk3
    pkgs.vte
  ];
}
