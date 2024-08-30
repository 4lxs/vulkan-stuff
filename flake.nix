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
            ninja
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
            pkgs.ninja
            pkgs.cmake
            pkgs.gcc
            pkgs.mesa

            pkgs.vulkan-headers
            pkgs.vulkan-loader
            pkgs.vulkan-tools
            pkgs.vulkan-validation-layers
            pkgs.vulkan-utility-libraries
            pkgs.shaderc
            pkgs.shaderc.bin
            pkgs.shaderc.static
            pkgs.shaderc.dev
            pkgs.shaderc.lib

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
            pkgs.pkg-config

            pkgs.SDL2
            pkgs.fmt
            pkgs.imgui
            pkgs.glm
            pkgs.tinyobjloader
            pkgs.stb
            pkgs.glslang
            pkgs.cmake-lint
            pkgs.cmake-format
          ];

          LD_LIBRARY_PATH = "${pkgs.vulkan-loader}/lib:${pkgs.shaderc.lib}/lib:${pkgs.shaderc.dev}/lib";
          VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
          VULKAN_LIB_DIR = "${pkgs.shaderc.dev}/lib";
        };
        formatter = pkgs.alejandra;
      };
    };
}
