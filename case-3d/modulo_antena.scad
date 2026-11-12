// modulo_antena.scad: módulo do NRF24L01 PA+LNA na posição de IMPRESSÃO.
// O SMA atravessa a ponta de fora; a antena rosqueia por fora.
// A fixação da placa do NRF dentro da caixa fica para depois de medir.
include <pecas.scad>

peca = "caixa";  // [caixa, tampa]

if (peca == "caixa") translate([0, 0, ant_prof()]) modulo_antena();
if (peca == "tampa") translate([0, 0, ant_prof()]) tampa_antena();
