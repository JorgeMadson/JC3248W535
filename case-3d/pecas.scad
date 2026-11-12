// pecas.scad: biblioteca com todas as peças, na posição de montagem.
// Não gera nada sozinho. Os arquivos de impressão (case_base, modulo_*, teste_encaixe)
// e a montagem incluem este arquivo e chamam as peças.
include <parametros.scad>

// ================= Espelho =================
// As posições (parametros.scad) estão na vista traseira do PDF, mas Z aponta para a
// tela. Sem espelhar, a peça sairia como imagem no espelho da placa real. Todas as
// peças são desenhadas nessas coordenadas (_nome) e publicadas espelhadas (nome).
module espelho() translate([placa_c, 0, 0]) mirror([1, 0, 0]) children();

// ================= Ajudantes =================
module caixa_arred(x0, y0, x1, y1, z0, z1, r) {
  rr = max(0.01, min(r, (x1 - x0) / 2 - 0.01, (y1 - y0) / 2 - 0.01));
  hull() for (x = [x0 + rr, x1 - rr], y = [y0 + rr, y1 - rr])
    translate([x, y, z0]) cylinder(r = rr, h = z1 - z0);
}

// Trilho macho: base em z = 0, cresce para +Z, corre em X de x0 até x1
module trilho_macho(x0, x1, y = trilho_y) {
  b = trilho_base / 2; t = trilho_topo / 2;
  translate([x0, y, 0]) rotate([90, 0, 90]) linear_extrude(x1 - x0)
    polygon([[-b, -0.01], [b, -0.01], [t, trilho_h], [-t, trilho_h]]);
}

// Canal fêmea: abre em z = 0 (face externa), entra para +Z
module trilho_femea(x0, x1, f = folga_trilho, y = trilho_y) {
  b = trilho_base / 2 + f; t = trilho_topo / 2 + f;
  translate([x0, y, 0]) rotate([90, 0, 90]) linear_extrude(x1 - x0)
    polygon([[-b, -1], [b, -1], [b, 0], [t, trilho_h + f], [-t, trilho_h + f], [-b, 0]]);
}

module trava_calombo(x, y = trilho_y)
  translate([x, y, trilho_h + trava_altura - trava_r]) sphere(r = trava_r);

module trava_covinha(x, f = folga_trilho, y = trilho_y)
  translate([x, y, trilho_h + f + trava_altura + 0.3 - (trava_r + 0.3)]) sphere(r = trava_r + 0.3);

// ================= Case base =================

// Moldura da frente: segura a placa pela borda da tela
module _moldura() {
  cx = placa_c / 2 + tela_dx; cy = placa_l / 2 + tela_dy;
  jc = tela_c + 2 * tela_margem; jl = tela_l + 2 * tela_margem;
  difference() {
    caixa_arred(-ext, -ext, placa_c + ext, placa_l + ext, z_placa, z_topo, ext_r);
    caixa_arred(-folga, -folga, placa_c + folga, placa_l + folga,
                z_placa - 1, z_placa + placa_e, placa_r + folga);
    translate([cx - jc / 2, cy - jl / 2, z_topo - esp_frente - 1]) cube([jc, jl, esp_frente + 2]);
  }
}

// Tampa traseira: parafusa na placa, tem as janelas dos conectores e o canal dos módulos
module _tampa() {
  difference() {
    union() {
      caixa_arred(-ext, -ext, placa_c + ext, placa_l + ext, 0, esp_tras, ext_r);
      difference() {   // saia em volta, fecha o vão até a moldura
        caixa_arred(-ext, -ext, placa_c + ext, placa_l + ext, esp_tras - 0.01, z_placa, ext_r);
        caixa_arred(-folga, -folga, placa_c + folga, placa_l + folga,
                    esp_tras - 1, z_placa + 1, placa_r + folga);
      }
      for (p = furos) translate([p[0], p[1], 0]) cylinder(d = torre_d, h = z_placa);
    }
    for (p = furos) translate([p[0], p[1], 0]) {
      translate([0, 0, -1]) cylinder(d = parafuso_furo, h = z_placa + 2);
      translate([0, 0, -1]) cylinder(d = parafuso_cab_d, h = parafuso_cab_h + 1);
    }
    for (c = conectores)
      translate([c[1] - c[3] / 2, c[2] - c[4] / 2, -1]) cube([c[3], c[4], z_placa + 2]);
    translate([porta_x - porta_c / 2, porta_y - porta_l / 2, -1]) cube([porta_c, porta_l, esp_tras + 2]);
    trilho_femea(-ext - 1, trilho_fim);
    trava_covinha(trilho_fim - trava_recuo);
  }
}

// ================= Módulos =================

// Corpo padrão de módulo: caixa de z = -prof até 0, com o trilho em cima
module modulo_corpo(prof) {
  caixa_arred(mod_x0, mod_y0, mod_x1, mod_y1, -prof, 0, mod_r);
  trilho_macho(mod_x0, trilho_fim - folga_trilho);
  trava_calombo(trilho_fim - trava_recuo);
}

module porta_furo(z0, z1)
  translate([porta_x - porta_c / 2, porta_y - porta_l / 2, z0]) cube([porta_c, porta_l, z1 - z0]);

// ---- Cartucho de bateria ----
function bat_dim(i) = [baterias[i][1] + 2 * folga_bat, baterias[i][2] + 2 * folga_bat,
                       baterias[i][3] + folga_bat];
function bat_prof(i) = esp_min + bat_dim(i)[2] + esp_min;   // teto + bateria + tampa
function bat_y0(i) = (mod_y0 + mod_y1) / 2 - bat_dim(i)[1] / 2;

module _cartucho_bateria(i) {
  d = bat_dim(i); prof = bat_prof(i);
  x0 = mod_x0 + esp_min;                 // bateria encostada na ponta de fora
  bx0 = mod_x1 - esp_min - baia_c;       // baia do adaptador, embaixo da porta
  difference() {
    modulo_corpo(prof);
    translate([x0, bat_y0(i), -prof - 1]) cube([d[0], d[1], d[2] + esp_min + 1]);
    translate([bx0, mod_y0 + esp_min, -prof - 1])
      cube([baia_c, mod_y1 - mod_y0 - 2 * esp_min, d[2] + esp_min + 1]);
    translate([x0 + d[0] - 1, (mod_y0 + mod_y1) / 2 - 4, -prof - 1])   // canal dos fios
      cube([bx0 - x0 - d[0] + 2, 8, d[2] + esp_min + 1]);
    porta_furo(-esp_min - 1, 1);
    translate([x0 + 3, mod_y0 + 6, -0.4])        // nome gravado em cima, fora do trilho
      mirror([1, 0, 0]) linear_extrude(1)   // desfaz o espelho() para o texto ficar legível
        text(i == 0 ? "JBL" : "1300", size = 5, halign = "right", valign = "center");
  }
}

// Tampa de pressão do cartucho (fecha por baixo)
module _tampa_cartucho(i) {
  prof = bat_prof(i); f = 0.15;
  x0 = mod_x0 + esp_min; bx0 = mod_x1 - esp_min - baia_c; d = bat_dim(i);
  color("silver") translate([0, 0, -prof]) union() {
    translate([x0 + f, bat_y0(i) + f, 0]) cube([d[0] - 2 * f, d[1] - 2 * f, esp_min]);
    translate([bx0 + f, mod_y0 + esp_min + f, 0])
      cube([baia_c - 2 * f, mod_y1 - mod_y0 - 2 * esp_min - 2 * f, esp_min]);
    translate([x0 + d[0] - 1 + f, (mod_y0 + mod_y1) / 2 - 4 + f, 0])
      cube([bx0 - x0 - d[0] + 2 - 2 * f, 8 - 2 * f, esp_min]);
  }
}

// ---- Módulo de antena (NRF24L01 PA+LNA) ----
function ant_prof() = esp_min + nrf_prof + esp_min;

module _modulo_antena() {
  prof = ant_prof();
  difference() {
    modulo_corpo(prof);
    translate([mod_x0 + esp_min, mod_y0 + esp_min, -prof - 1])
      cube([mod_x1 - mod_x0 - 2 * esp_min, mod_y1 - mod_y0 - 2 * esp_min, nrf_prof + esp_min + 1]);
    translate([mod_x0 - 1, (mod_y0 + mod_y1) / 2, -prof + esp_min + sma_z])   // SMA sai pela ponta
      rotate([0, 90, 0]) cylinder(d = sma_d, h = esp_min + 2);
    porta_furo(-esp_min - 1, 1);
  }
}

module _tampa_antena() {
  prof = ant_prof(); f = 0.15;
  color("silver") translate([mod_x0 + esp_min + f, mod_y0 + esp_min + f, -prof])
    cube([mod_x1 - mod_x0 - 2 * esp_min - 2 * f, mod_y1 - mod_y0 - 2 * esp_min - 2 * f, esp_min]);
}

// ================= Fantasmas (só para visualizar) =================
module _placa_fantasma() {
  color([0.12, 0.12, 0.13]) difference() {
    caixa_arred(0, 0, placa_c, placa_l, z_placa, z_placa + placa_e, placa_r);
    for (p = furos) translate([p[0], p[1], z_placa - 1]) cylinder(d = 2, h = 4);
  }
  color([0.15, 0.35, 0.75])
    translate([placa_c / 2 + tela_dx - tela_c / 2, placa_l / 2 + tela_dy - tela_l / 2, z_placa + placa_e])
      cube([tela_c, tela_l, 0.2]);
  for (c = conectores) color(c[0] == "USB-C" ? "silver" : "white")
    translate([c[1] - c[5] / 2, c[2] - c[6] / 2, z_placa - 1.5]) cube([c[5], c[6], 1.5]);
}

module _bateria_fantasma(i) {
  d = bat_dim(i);
  color([0.85, 0.6, 0.1])
    translate([mod_x0 + esp_min + folga_bat, bat_y0(i) + folga_bat, -bat_prof(i) + esp_min])
      cube([baterias[i][1], baterias[i][2], baterias[i][3]]);
}

module _nrf_fantasma() {
  prof = ant_prof(); y = (mod_y0 + mod_y1) / 2;
  color([0.1, 0.45, 0.2]) translate([mod_x0 + esp_min + 8, y - nrf_l / 2, -prof + esp_min + sma_z - nrf_e / 2])
    cube([nrf_c, nrf_l, nrf_e]);
  color("gold") translate([mod_x0 + esp_min + 8, y, -prof + esp_min + sma_z]) rotate([0, -90, 0])
    cylinder(d = 6.35, h = 8 + esp_min + 6);
  color([0.1, 0.1, 0.1]) translate([mod_x0 - 6, y, -prof + esp_min + sma_z]) rotate([0, -90, 0])
    cylinder(d = 9.5, h = 109);   // antena [NET] 109 x 9,5 mm, reta
}

// ================= Peças publicadas (já espelhadas) =================
module moldura() espelho() _moldura();
module tampa() espelho() _tampa();
module cartucho_bateria(i) espelho() _cartucho_bateria(i);
module tampa_cartucho(i) espelho() _tampa_cartucho(i);
module modulo_antena() espelho() _modulo_antena();
module tampa_antena() espelho() _tampa_antena();
module placa_fantasma() espelho() _placa_fantasma();
module bateria_fantasma(i) espelho() _bateria_fantasma(i);
module nrf_fantasma() espelho() _nrf_fantasma();
