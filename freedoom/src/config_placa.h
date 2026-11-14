#pragma once
// Pinos e ajustes da JC3248W535 para o Freedoom (firmware próprio, Arduino).
// Nada de pino fora daqui. O launcher tem o seu config_placa.h.

// ---------- Orientação ----------
// A tela é usada deitada (480x320). 1 ou 3: escolhe qual lado fica para cima.
// Se a imagem aparecer de cabeça para baixo, troque.
#define ROTACAO_TELA 1

// 1 = desenha os contornos dos botões de toque por cima do jogo.
#define MOSTRAR_BOTOES 1

// ---------- Tela (QSPI) ----------
#define PINO_LCD_CS 45
#define PINO_LCD_SCK 47
#define PINO_LCD_D0 21
#define PINO_LCD_D1 48
#define PINO_LCD_D2 40
#define PINO_LCD_D3 39
#define PINO_LCD_BL 1  // backlight, HIGH = liga

// ---------- Toque (I2C) ----------
#define PINO_TOQUE_SDA 4
#define PINO_TOQUE_SCL 8
#define ENDERECO_TOQUE 0x3B
// Dedos lidos por vez. Com 2 dá para andar e atirar ao mesmo tempo.
// Se o toque ficar estranho, volte para 1 (leitura igual à do placar-mesa).
#define TOQUE_MAX_PONTOS 2

// ---------- Som (I2S, conector P6) ----------
#define PINO_I2S_BCLK 42
#define PINO_I2S_LRCLK 2
#define PINO_I2S_DADOS 41
#define SOM_TAXA_HZ 22050
// Volume geral, 0 a 256 (256 = sem atenuar).
#define SOM_GANHO 160

// ---------- microSD (SPI). Pinos do esquema, ainda não testados ----------
#define PINO_SD_CS 10
#define PINO_SD_MOSI 11
#define PINO_SD_SCK 12
#define PINO_SD_MISO 13

// ---------- Imagem da tela (ver comum/i_video.c) ----------
// O Arduino_Canvas tem a forma do painel (em pé) e vai direto para ele.
#define FB_LARGURA PAINEL_LARGURA
#define FB_ALTURA PAINEL_ALTURA
#define FB_GIRO ROTACAO_TELA
#define FB_TROCA_BYTES 0

// ---------- Tamanhos ----------
#define TELA_LARGURA 480  // deitada
#define TELA_ALTURA 320
#define PAINEL_LARGURA 320  // como o painel é de verdade (em pé)
#define PAINEL_ALTURA 480
