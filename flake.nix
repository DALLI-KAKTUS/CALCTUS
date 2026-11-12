{
  description = "CALCTUS: açık kaynak hesap makinesi yazılımı (masaüstü simülatör + ESP32-S3)";

  # nixos-26.05; sistem flake'iyle (~/Documents/nix) aynı sürüm, tekrar indirme olmaz.
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/718f670e845328e9c2cecdd832835712b2cbd065";

  outputs = { self, nixpkgs }:
    let
      systems = [ "aarch64-darwin" "x86_64-darwin" "x86_64-linux" "aarch64-linux" ];
      forAll = f: nixpkgs.lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});
    in {
      devShells = forAll (pkgs: {
        default = pkgs.mkShell {
          packages = [
            pkgs.cmake
            pkgs.ninja
            pkgs.pkg-config
            # Yalnız simülatör penceresi için; ortak kod SDL'i görmez.
            pkgs.SDL2
            # Yalnız derleme araçları için (yazı tipi üretimi, testler).
            (pkgs.python3.withPackages (ps: [ ps.pillow ps.fonttools ]))
          ];
          # tools/fontgen.py yazı tiplerini buradan okur.
          CALCTUS_FONT_DIR = "${pkgs.dejavu_fonts}/share/fonts/truetype";
        };
      });
    };
}
