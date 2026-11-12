// parametros.scad — TODAS as medidas da case modular da JC3248W535.
// Os outros arquivos só usam o que está aqui. Para ajustar depois de um teste, mude aqui.
//
// ETIQUETAS (leia antes de imprimir para valer):
//   [NET]  número publicado (fabricante ou vendedor; fonte ao lado). Conferir no paquímetro.
//   [PDF~] estimado medindo os pixels do desenho do PDF da Guition. Erro de ~±1 mm.
//   [SUP]  suposição sem fonte nenhuma. A MEDIR.
//   [PROJ] decisão de projeto: pode mudar à vontade.
//
// PDF da Guition: https://done.land/assets/files/jc3248w535%20specifications-en.pdf
//   pág. 4 (tabela) e 6 (desenho): módulo 94,5 x 62,0 mm, área visível 73,4 x 49,0 mm.
//   pág. 5: vista traseira com os conectores. As posições [PDF~] vêm dela
//   (escala: 504 px = 94,5 mm, ou seja, 5,33 px/mm).
//
// COORDENADAS (iguais em todos os arquivos):
//   X, Y = placa vista POR TRÁS, como na pág. 5 do PDF: USB-C à direita, origem no
//          canto inferior esquerdo da placa.
//   Z    = para cima, na direção da tela. Z = 0 é a face externa da tampa traseira.

$fn = $preview ? 32 : 64;

// ---------------- Impressão [PROJ] ----------------
folga        = 0.30;  // entre a placa e a moldura, por lado
folga_trilho = 0.25;  // rabo de andorinha, por lado: escolher com teste_encaixe.scad
parede       = 2.0;   // parede lateral (5 linhas de bico 0,4)
esp_min      = 1.6;   // pisos e tetos finos (8 camadas de 0,2 mm)

// ---------------- Placa JC3248W535 ----------------
placa_c = 94.5;   // [NET] PDF: "Module size 94.5*62.0(mm)"
placa_l = 62.0;   // [NET] idem
placa_e = 11.0;   // [SUP] ESPESSURA TOTAL (vidro até as costas). Nenhuma fonte dá. A MEDIR
placa_r = 4.0;    // [SUP] raio dos cantos
tela_c  = 73.4;   // [NET] PDF: "Effective display area 73.4*49.0(mm)"
tela_l  = 49.0;   // [NET]
tela_dx = 0;      // [SUP] deslocamento do centro da tela (no PDF parece centrada)
tela_dy = 0;      // [SUP]
tela_margem = 1.0; // [PROJ] abertura da moldura maior que a área visível, por lado

// Furos dos 4 cantos (nas costas)
furo_mx = 5.0;    // [PDF~] da borda ao centro do furo, em X (pixels: 5,1 e 5,0)
furo_my = 5.0;    // [PDF~] idem em Y (pixels: 5,3 e 4,9)
parafuso_furo  = 2.4;  // [SUP] passagem para M2. Nem a rosca dos furos é conhecida
parafuso_cab_d = 4.6;  // [SUP] cabeça de M2 + folga
parafuso_cab_h = 1.8;  // [SUP]

// Conectores e botões nas costas (pág. 5 do PDF)
// [nome, centro x, centro y, janela x, janela y, peça x, peça y]
//   centro e peça = [PDF~]; janela = peça + espaço para o plugue/dedo = [SUP]
conectores = [
  ["P5 bateria",    70.4, 55.2,  9,  7,  3.6,  3.7],
  ["chave bateria", 77.9, 55.5,  5,  5,  2.0,  2.0],
  ["USB-C",         82.6, 36.0,  9, 14,  5.6, 10.7], // janela = capa do plugue do cabo [SUP]
  ["P1",            82.0, 25.9,  7, 13,  4.0,  9.8],
  ["P4 QWIIC",      17.6,  6.9, 11,  7,  7.5,  3.8],
  ["P3",            29.1,  6.9, 11,  7,  7.9,  3.8],
  ["P2",            51.4,  6.9, 16,  7, 12.2,  3.8],
  ["P6 som",        69.4,  6.9,  9,  7,  5.4,  3.8],
  ["Boot/Reset",    12.2, 12.9,  9,  5,  6.0,  2.0],
];
// microSD: não aparece no PDF (deve ficar numa borda). Fica sem recorte até medir.

// ---------------- Case base [PROJ] ----------------
folga_tras = 4.0;  // [SUP] vão entre as costas da placa e a tampa (torres + fio interno)
esp_frente = 1.6;  // borda da moldura por cima da tela
torre_d    = 6.5;  // torres dos parafusos

// Trilho rabo de andorinha (macho no módulo, fêmea na tampa)
trilho_base = 8.0;  // largura do macho na base
trilho_h    = 3.0;  // altura
trilho_ang  = 30;   // flanco em relação à vertical (<= 45 imprime sem suporte)
trilho_y    = 31.0; // centro do trilho em Y (meio da placa)
trilho_fim  = 75.0; // batente, antes da área do USB-C/P1
// Trava de pressão: calombo no macho, covinha no teto da fêmea
trava_altura = 0.4; // quanto o calombo sobe acima do trilho: testar
trava_r      = 1.2;
trava_recuo  = 6;   // distância do batente até a trava

// Área que um módulo ocupa nas costas (livra P5 em cima e a fileira de baixo)
mod_y0 = 11.0;
mod_y1 = 51.0;
mod_r  = 2.0;

// Porta do módulo: abertura em posição FIXA, igual em todos os módulos
porta_x = 68;
porta_y = 44;
porta_c = 12;  // [SUP] tamanho de um conector magnético pogo de 2 pinos: trocar pela peça
porta_l = 6;

// ---------------- Baterias ----------------
// [nome, comprimento, largura, espessura]: [NET] da etiqueta/INVENTARIO
baterias = [
  ["JBL GSP383562",  62, 35, 3.8],
  ["Rontek 903048",  48, 30, 9.0],
];
folga_bat = 0.5;   // [PROJ] LiPo incha um pouco: folga generosa
baia_c    = 11;    // [SUP] espaço para o adaptador (conector original -> 2 fios)

// ---------------- NRF24L01 PA+LNA ----------------
nrf_c   = 41.0;  // [NET] vendedores dão 37 a 45,5 mm, sem o SMA. A MEDIR
nrf_l   = 15.5;  // [NET] idem 15 a 16,6
nrf_e   = 1.6;   // [SUP] espessura da placa
nrf_prof = 12;   // [SUP] altura interna (componentes + pinos + capacitor)
sma_d   = 6.8;   // [PROJ] rosca SMA = 1/4" (6,35 mm) + folga
sma_z   = 4.0;   // [SUP] centro do SMA acima do fundo da cavidade

// ---------------- Derivados (não mexer) ----------------
ext      = folga + parede;                          // borda da placa -> face externa
ext_r    = placa_r + ext;
trilho_topo = trilho_base + 2 * trilho_h * tan(trilho_ang);
esp_tras = trilho_h + folga_trilho + esp_min;       // laje da tampa
z_placa  = esp_tras + folga_tras;                   // costas da placa
z_topo   = z_placa + placa_e + esp_frente;          // frente da moldura
mod_x0   = -ext;                                    // módulo rente à borda da case
mod_x1   = trilho_fim;
furos = [[furo_mx, furo_my], [placa_c - furo_mx, furo_my],
         [furo_mx, placa_l - furo_my], [placa_c - furo_mx, placa_l - furo_my]];
