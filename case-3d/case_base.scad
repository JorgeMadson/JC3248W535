// case_base.scad: peças da case base já na posição de IMPRESSÃO.
// Exportar: openscad -D 'peca="moldura"' -o moldura.stl case_base.scad
//   moldura: frente da tela virada para a mesa. Sem suporte.
//   tampa:   face externa (com o canal do trilho) na mesa. O teto do canal é uma
//            ponte de ~12 mm: sem suporte.
include <pecas.scad>

peca = "moldura"; // [moldura, tampa]

if (peca == "moldura") translate([0, 0, z_topo]) rotate([180, 0, 0]) moldura();
if (peca == "tampa") tampa();
