{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    miniCompileCommands = {
      url = "github:danielbarter/mini_compile_commands/v0.6";
      flake = false;
    };
    koturNixPkgs = {
      url = "github:nkoturovic/kotur-nixpkgs/v0.6.0";
      flake = false;
    };
    systems.url = "github:nix-systems/default";
  };
  outputs = {
    self,
    flake-parts,
    systems,
    miniCompileCommands,
    ...
  } @ inputs:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = import systems;
      flake = {
      };
      perSystem = {
        pkgs,
        system,
        config,
        ...
      }: let
        mcc-env = (pkgs.callPackage miniCompileCommands {}).wrap pkgs.stdenv;
        mcc-hook = (pkgs.callPackage miniCompileCommands {}).hook;
      in {
        packages.default = mcc-env.mkDerivation (self: {
          name = "vulkan";
          version = "0.0.1";

          nativeBuildInputs = with pkgs; [
            mcc-hook

            cmake
            gnumake

            vulkan-headers
            vulkan-loader
            vulkan-validation-layers
            libGLU

            glfw
            glm
            shaderc
          ];

          buildInputs = with pkgs; [
            fmt
          ];

          LD_LIBRARY_PATH = "${pkgs.glfw}/lib:${pkgs.freetype}/lib:${pkgs.vulkan-loader}/lib:${pkgs.vulkan-validation-layers}/lib";

          src = builtins.path {
            path = ./.;
            filter = path: type:
              !(pkgs.lib.hasPrefix "." (baseNameOf path));
          };

          cmakeFlags = [
            "--no-warn-unused-cli"
          ];
        });
        devShells.default = (pkgs.mkShell.override {stdenv = mcc-env;}) {
          buildInputs = [
            pkgs.cmake
            pkgs.gcc
            pkgs.mesa
            pkgs.vulkan-headers
            pkgs.vulkan-loader
            pkgs.vulkan-tools
            pkgs.vulkan-validation-layers
            pkgs.shaderc
            pkgs.shaderc.bin
            pkgs.shaderc.static
            pkgs.shaderc.dev
            pkgs.shaderc.lib
            #pkgs.glslang
            pkgs.glfw
            pkgs.glm
            pkgs.libGLU
            pkgs.python3
            pkgs.xorg.libX11
            pkgs.xorg.libXcursor
            pkgs.xorg.libXrandr
            pkgs.xorg.libXinerama
            pkgs.xorg.libXi
            pkgs.xorg.libXext
            pkgs.xorg.libXrender
            pkgs.xorg.libXxf86vm
            pkgs.xorg.libXdmcp
            pkgs.xorg.libXau
            pkgs.xorg.libxcb
            pkgs.gnutls
            pkgs.xorg.libpthreadstubs
            pkgs.llvm_15
            pkgs.clang_15
            pkgs.pkg-config

            pkgs.fmt
            pkgs.imgui
            pkgs.glm
            pkgs.tinyobjloader
            pkgs.stb
            pkgs.glslang
          ];

          LD_LIBRARY_PATH = "${pkgs.vulkan-loader}/lib:${pkgs.shaderc.lib}/lib:${pkgs.shaderc.dev}/lib";
          VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
          VULKAN_LIB_DIR = "${pkgs.shaderc.dev}/lib";
        };
        formatter = pkgs.alejandra;
      };
    };
}
