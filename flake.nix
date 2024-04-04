{
  description = "SpeakEasy 2 R package";

  inputs = { nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.11"; };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      REnv = pkgs.rWrapper.override {
        packages = with pkgs.rPackages; [ styler lintr devtools igraph ];
      };
    in {
      devShells.${system}.default = pkgs.mkShell {
        packages = [ REnv ] ++ (with pkgs; [
          astyle
          cmake
          ninja
          gdb
          # igraph dependencies
          bison
          flex
          libxml2
        ]);
        shellHook = ''
          export OMP_NUM_THREADS=16
        '';
      };
    };
}
