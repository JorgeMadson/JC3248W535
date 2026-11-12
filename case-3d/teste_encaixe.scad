// teste_encaixe.scad: teste rápido (~15 min) do rabo de andorinha e da trava.
// Bloco com 3 canais fêmea (folgas diferentes, gravadas em cima) + 1 chave macho.
// Imprima, deslize a chave em cada canal e escolha a folga que corre sem jogo e
// "clica" na trava. Depois ponha esse valor em folga_trilho (parametros.scad).
// As duas peças imprimem sem suporte, na posição em que aparecem aqui.
include <pecas.scad>

folgas = [0.15, 0.25, 0.35];
compr = 20;   // comprimento dos canais
passo = 18;   // distância entre canais

// Bloco fêmea (abertura dos canais para baixo, como na tampa de verdade)
difference() {
  cube([compr, passo * len(folgas), esp_tras]);
  for (i = [0 : len(folgas) - 1]) {
    y = passo / 2 + i * passo;
    trilho_femea(-1, compr + 1, folgas[i], y);
    trava_covinha(compr / 2, folgas[i], y);
    translate([compr / 2, y, esp_tras - 0.6]) linear_extrude(1)
      text(str(folgas[i]), size = 3.5, halign = "center", valign = "center");
  }
}

// Chave macho (uma só serve para os 3 canais)
translate([0, passo * len(folgas) + 14, 0]) {
  ch = 30;
  translate([0, -8, 0]) cube([ch, 16, esp_min]);
  translate([0, 0, esp_min]) {
    trilho_macho(0, ch, 0);
    trava_calombo(ch / 2, 0);
  }
}
