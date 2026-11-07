#pragma once
// Pinos e ajustes de hardware da JC3248W535. Nada de pino fora de src/hal/.

// ---------- Depuração ----------
// 1 = desenha uma bolinha onde o toque foi sentido e imprime na serial.
#define DEBUG_TOQUE 0

// ---------- Ajustes do toque (se um dia sair espelhado) ----------
#define TOQUE_TROCA_XY 0
#define TOQUE_INVERTE_X 0
#define TOQUE_INVERTE_Y 0

// ---------- Pinos ----------
#define PINO_LCD_CS 45
#define PINO_LCD_SCK 47
#define PINO_LCD_D0 21
#define PINO_LCD_D1 48
#define PINO_LCD_D2 40
#define PINO_LCD_D3 39
#define PINO_LCD_BL 1  // backlight, HIGH = liga
#define PINO_TOQUE_SDA 4
#define PINO_TOQUE_SCL 8
#define ENDERECO_TOQUE 0x3B
