{
  makePlatformIOSetupHook,
  platformio,
  stdenv,
}: let
  setupHook = makePlatformIOSetupHook {
    lockfile = ./platformio2nix.lock;
    overrides = final: prev: {
      "packages/toolchain-gccarmnoneeabi" = prev."packages/toolchain-gccarmnoneeabi".overrideAttrs (drv: {
        dontFixup = true; # dunno why, but this breaks things
      });
    };
  };
in
  stdenv.mkDerivation {
    name = "simplefoc-driveshield-atmega2560";
    version = "0.0.0";
    src = ./.;
    nativeBuildInputs = [
      platformio
      setupHook
      # (python3.withPackages (python-pkgs:
      #   with python-pkgs; [
      #     pip
      #     pycryptodome
      #   ]))
    ];

    buildPhase = ''
      pio run
    '';

    installPhase = ''
      mv .pio/build/pico $out
    '';

    passthru = { inherit setupHook; };
  }
