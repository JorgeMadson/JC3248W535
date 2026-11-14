#pragma once
// Ponte em C entre o motor do Doom (C) e o hardware da placa (C++/Arduino).
// O motor não inclui nada do Arduino e o código Arduino não inclui nada do Doom
// (o `boolean` dos dois tem tamanho diferente).
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Liga tela, toque, cartão e a partição de saves. Chamar uma vez.
void placa_iniciar(void);

// Imagem da tela: FB_LARGURA x FB_ALTURA em RGB565 (ver config_placa.h).
uint16_t *placa_fb(void);
// Manda a imagem inteira para a tela.
void placa_enviar(void);

// Contornos dos botões de toque. modo: 0 = menu, 1 = jogo.
void placa_desenhar_botoes(int modo);

// Texto de inicialização na tela (para de desenhar depois de placa_console_fim).
void placa_console(const char *texto, int tamanho);
void placa_console_fim(void);

// Tela de erro. Não envia: chame placa_enviar depois.
void placa_tela_erro(const char *titulo, const char *msg);

// Lê os dedos na tela, já em coordenadas da tela deitada (0..479, 0..319).
// (o desenho dos botões em placa_desenhar_botoes usa as mesmas áreas de entrada.c)
// Devolve quantos (0 a 2).
int placa_ler_toques(int16_t x[2], int16_t y[2]);

// Onde está o jogo: caminho do WAD ("/sd/..." ou "/flash/..."), ou NULL.
const char *placa_caminho_wad(void);
// 1 se a partição `wad` da flash tem um WAD gravado.
int placa_wad_na_flash(void);

// Saída de som em 16 bits estéreo intercalado (L, R, L, R...). iniciar devolve
// 0 se não der; escrever bloqueia até caber (é o que dá o ritmo do mixer).
int placa_som_iniciar(int taxa_hz);
void placa_som_escrever(const int16_t *estereo, int quadros);

uint32_t placa_millis(void);
void placa_reiniciar(void);

#ifdef __cplusplus
}
#endif
