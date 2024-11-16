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
      }: {
        _module.args.pkgs = import inputs.nixpkgs {
          inherit system;
          overlays = [inputs.platformio2nix.overlays.default];
        };
        packages.default = pkgs.callPackage ./package.nix {};
        make-shells.default = {
          imports = [
            ({pkgs, ...}: {
              config.packages = [
                pkgs.platformio
                pkgs.ccls
                pkgs.rlwrap
                (pkgs.python3.withPackages
                  (ps: [ps.pyserial]))
                (pkgs.writeShellScriptBin "miniterm" ''
                  ${pkgs.python3.withPackages (ps: [ps.pyserial])}/bin/python3 -m serial.tools.miniterm "$@"
                '')
                (pkgs.writeShellScriptBin "monitor" ''
                  ${pkgs.python3.withPackages (ps: [ps.pyserial])}/bin/python3 ./serial_script.py "$@"
                '')
                (pkgs.writeShellScriptBin "pio-build-flash-monitor" ''
                  pio run -t upload
                  monitor
                  # pio device monitor --echo
                '')
              ];
              config.shellHook = ''
                pio project init --ide vim --board megaatmega2560
                echo "Initialized project (so that \`ccls\` works correctly)..."
                echo "Note: this creates a \`.ccls\` directory in the project that is not tracked by git."
                echo "This file must be regenerated each time you change the absolute path of any source files,
                  or add any new dependencies. Otherwise, \`ccls\` may not be able to perform exhaustive code analysis.
                "
              '';
            })
          ];
        };
      };

      flake = {
      };
    };
}
