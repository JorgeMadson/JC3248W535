// montagem.scad: SÓ PARA VISUALIZAR (não é para imprimir).
// Mostra a case base com um módulo encaixado; "explodir" separa as peças.
include <pecas.scad>

modulo   = "bateria_jbl"; // [bateria_jbl, bateria_rontek, antena, nenhum]
explodir = 0;              // mm entre as peças (0 = montado)
corte    = false;          // true = corta ao meio para ver por dentro

module tudo() {
  translate([0, 0, 2 * explodir]) color([0.25, 0.25, 0.28]) moldura();
  translate([0, 0, explodir]) placa_fantasma();
  color([0.9, 0.9, 0.92]) tampa();
  translate([-explodir * 1.5, 0, -explodir]) {
    if (modulo == "bateria_jbl")    { color([0.95, 0.45, 0.1]) cartucho_bateria(0); tampa_cartucho(0); bateria_fantasma(0); }
    if (modulo == "bateria_rontek") { color([0.95, 0.45, 0.1]) cartucho_bateria(1); tampa_cartucho(1); bateria_fantasma(1); }
    if (modulo == "antena")         { color([0.2, 0.6, 0.9]) modulo_antena(); tampa_antena(); nrf_fantasma(); }
  }
}

if (corte) difference() { tudo(); translate([-200, trilho_y, -100]) cube([400, 200, 200]); }
else tudo();
