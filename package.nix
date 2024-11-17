{
  makePlatformIOSetupHook,
  platformio,
  stdenv,
}: let
  setupHook = makePlatformIOSetupHook {
    lockfile = ./platformio2nix.lock;
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
      mv .pio/build/megaatmega2560 $out
    '';
  }
