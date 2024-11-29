{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    systems.url = "github:nix-systems/default";
  };
  outputs = {
    self,
    flake-parts,
    systems,
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
        llvm = pkgs.llvmPackages_19;
      in {
        devShells.default = (pkgs.mkShell.override {inherit (llvm) stdenv;}) {
          nativeBuildInputs = [
            pkgs.cmake
            pkgs.ninja

            # clang-tools must be above stdlib because ... reasons
            # https://discourse.nixos.org/t/clang-tidy-doesnt-find-stdlib/37641/3
            llvm.clang-tools
            llvm.libcxx
            llvm.libllvm
          ];

          buildInputs = [
            pkgs.pkg-config

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

            pkgs.SDL2
            # pkgs.fmt
            # pkgs.imgui
            pkgs.glm
            pkgs.tinyobjloader
            pkgs.stb
            pkgs.glslang
            pkgs.wayland
            pkgs.wayland-scanner
            pkgs.egl-wayland
            pkgs.libxkbcommon
            pkgs.libffi
            pkgs.gcc
            pkgs.spirv-headers

            config.formatter
            pkgs.clang-tools
            pkgs.cmake-lint
            pkgs.cmake-format
            pkgs.doxygen
            pkgs.ccache
            pkgs.cppcheck
            pkgs.include-what-you-use
            pkgs.cmakeCurses
          ];

          LD_LIBRARY_PATH = "${pkgs.vulkan-loader}/lib:${pkgs.shaderc.lib}/lib:${pkgs.shaderc.dev}/lib:${pkgs.wayland}/lib:${pkgs.libxkbcommon}/lib:${pkgs.libffi}/lib";
          VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
          VULKAN_LIB_DIR = "${pkgs.shaderc.dev}/lib";
        };
        formatter = pkgs.alejandra;
      };
    };
}
