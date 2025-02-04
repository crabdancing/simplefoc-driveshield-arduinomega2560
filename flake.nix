{
  description = "simplefoc-driveshield-mega2560";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    make-shell.url = "github:nicknovitski/make-shell";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    platformio2nix.url = "github:nathanregner/platformio2nix";
  };

  outputs = inputs @ {flake-parts, ...}:
    flake-parts.lib.mkFlake {inherit inputs;} {
      imports = [
        inputs.make-shell.flakeModules.default
      ];

      systems = ["x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin"];

      perSystem = {
        config,
        self',
        inputs',
        pkgs,
        system,
        ...
      }: let
        simple-monitor = pkgs.writeShellScriptBin "simple-monitor" ''
          ${pkgs.python3.withPackages (ps: [ps.pyserial ps.colorama])}/bin/python3 ./simple-monitor.py "$@"
        '';
      in {
        _module.args.pkgs = import inputs.nixpkgs {
          inherit system;
          overlays = [inputs.platformio2nix.overlays.default];
        };
        packages.default = pkgs.callPackage ./package.nix {};
        packages.simple-monitor = simple-monitor;
        make-shells.default = {
          imports = [
            ({pkgs, ...}: {
              config.packages = [
                pkgs.platformio
                pkgs.clang-tools
                pkgs.rlwrap
                (pkgs.python3.withPackages
                  (ps: [ps.pyserial]))
                (pkgs.writeShellScriptBin "miniterm" ''
                  ${pkgs.python3.withPackages (ps: [ps.pyserial])}/bin/python3 -m serial.tools.miniterm "$@"
                '')
                (pkgs.writeShellScriptBin "pio-build-flash-monitor" ''
                  pio run -t upload
                  ${simple-monitor}/bin/simple-monitor
                  # pio device monitor --echo
                '')
              ];
              config.env = {
                BUILD_FLAGS = "-mfloat-abi=soft";
                PLATFORMIO_CORE_DIR = ".pio";
                PLATFORMIO_WORKSPACE_DIR = ".pio";
              };
              config.shellHook = ''
                # pio run -t compiledb
                # pio project init --ide vim --board pico
                # echo "Initialized project (so that \`ccls\` works correctly)..."
                # echo "Note: this creates a \`.ccls\` directory in the project that is not tracked by git."
                # echo "This file must be regenerated each time you change the absolute path of any source files,
                #   or add any new dependencies. Otherwise, \`ccls\` may not be able to perform exhaustive code analysis.
                # "
              '';
            })
          ];
        };
      };

      flake = {
      };
    };
}
