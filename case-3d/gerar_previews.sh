#!/usr/bin/env bash
# Gera as imagens de previews/. No Steam Deck (Flatpak) o OpenSCAD precisa do X11
# liberado para renderizar PNG; sem isso ele grava arquivos vazios.
set -e
cd "$(dirname "$0")"
OS="flatpak run --socket=x11 --device=dri --env=DISPLAY=${DISPLAY:-:0} org.openscad.OpenSCAD"
P="--imgsize=1400,1000 --colorscheme=Tomorrow --viewall --autocenter --projection=p"
img() { local nome=$1 arq=$2; shift 2; echo "== $nome"; $OS $arq $P "$@" -o "previews/$nome.png" 2>&1 | grep -iE "error|warn" || true; }
img 01_frente_montada      montagem.scad      --camera=0,0,0,55,0,25,0
img 02_tras_bateria_jbl    montagem.scad      --camera=0,0,0,125,0,200,0
img 03_explodida_jbl       montagem.scad      -D explodir=25 --camera=0,0,0,110,0,215,0
img 04_explodida_rontek    montagem.scad      -D explodir=25 -D 'modulo="bateria_rontek"' --camera=0,0,0,110,0,215,0
img 05_tras_antena         montagem.scad      -D 'modulo="antena"' --camera=0,0,0,120,0,210,0
img 07_tampa_janelas       montagem.scad      -D 'modulo="nenhum"' --camera=0,0,0,0,180,0,0
img 08_teste_encaixe       teste_encaixe.scad --camera=0,0,0,60,0,30,0
