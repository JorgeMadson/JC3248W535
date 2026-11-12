#!/usr/bin/env bash
# Gera todos os STL em case-3d/stl/ (fora do git). Uso: ./gerar_stl.sh
set -e
cd "$(dirname "$0")"
mkdir -p stl
gera() { local nome=$1 arq=$2; shift 2; echo "== $nome"; openscad "$@" -o "stl/$nome.stl" "$arq" 2>&1 | grep -iE "warn|error|Volumes" || true; }
gera moldura               case_base.scad      -D 'peca="moldura"'
gera tampa                 case_base.scad      -D 'peca="tampa"'
gera cartucho_jbl          modulo_bateria.scad -D bateria=0
gera tampa_cartucho_jbl    modulo_bateria.scad -D bateria=0 -D 'peca="tampa"'
gera cartucho_rontek       modulo_bateria.scad -D bateria=1
gera tampa_cartucho_rontek modulo_bateria.scad -D bateria=1 -D 'peca="tampa"'
gera modulo_antena         modulo_antena.scad
gera tampa_antena          modulo_antena.scad  -D 'peca="tampa"'
gera teste_encaixe         teste_encaixe.scad
