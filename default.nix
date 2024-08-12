{
  system ? builtins.currentSystem,
  lock ? builtins.fromJSON (builtins.readFile ./flake.lock),
  # The official nixpkgs input, pinned with the hash defined in the flake.lock file
  pkgs ? (import <nixpkgs> {}),
  # Helper tool for generating compile-commands.json
  miniCompileCommands ?
    fetchTarball {
      url = "https://github.com/danielbarter/mini_compile_commands/archive/${lock.nodes.miniCompileCommands.locked.rev}.tar.gz";
      sha256 = lock.nodes.miniCompileCommands.locked.narHash;
    },
}: let

  # Development shell
  shell = (pkgs.mkShell.override {stdenv = mcc-env;}) {
    # Copy build inputs (dependencies) from the derivation the nix-shell environment
    # That way, there is no need for speciying dependenvies separately for derivation and shell
    inputsFrom = [
      package
    ];

    # Shell (dev environment) specific packages
    packages = with pkgs; [
    ];
  };
in
  package
