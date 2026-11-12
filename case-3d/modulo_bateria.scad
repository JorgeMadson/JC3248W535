// modulo_bateria.scad: cartucho de bateria na posição de IMPRESSÃO.
//   cartucho: trilho para cima, fundo aberto na mesa. Sem suporte.
//   tampa:    placa fina, fecha o fundo por pressão.
// Exportar: openscad -D bateria=1 -D 'peca="cartucho"' -o cartucho_rontek.stl modulo_bateria.scad
include <pecas.scad>

bateria = 0;          // 0 = JBL GSP383562, 1 = Rontek 903048
peca = "cartucho";    // [cartucho, tampa]

if (peca == "cartucho") translate([0, 0, bat_prof(bateria)]) cartucho_bateria(bateria);
if (peca == "tampa") translate([0, 0, bat_prof(bateria)]) tampa_cartucho(bateria);
