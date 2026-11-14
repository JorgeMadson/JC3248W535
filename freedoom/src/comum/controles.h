#pragma once
// Lógica dos controles por toque, sem hardware: dedos na tela -> teclas do Doom.
// entrada.c lê o toque, chama ctrl_passo e entrega o resultado ao motor.
// Testada no PC em test/test_controles.
#include <stdint.h>

#define CTRL_MAX_EVENTOS 24

typedef struct
{
    // Estado entre uma leitura e a próxima (zere com memset antes da primeira).
    int segurada[8];
    int alavanca_ativa;
    int centro_x, centro_y;
    int ultimo_ax, ultimo_ay;
    uint32_t visto[8];  // quando cada área foi vista com um dedo pela última vez
    int ativa[8];       // a área está com um dedo (mesmo numa falha curta da tela)
    int antes_menu, antes_mapa, antes_arma, antes_direita;
    int dir_menu;
    uint32_t dir_menu_quando;
    int modo_menu_antes;
} ctrl_estado_t;

typedef struct
{
    int n_eventos;
    struct
    {
        unsigned char pressionada, tecla;
    } ev[CTRL_MAX_EVENTOS];
    int proxima_arma;  // 1: trocar para a próxima arma
} ctrl_saida_t;

// Uma leitura do toque. x/y: dedos em coordenadas da tela deitada (480x320), n de 0 a 2.
// no_menu: o menu do jogo está aberto. pergunta: há uma pergunta de sim/não na tela.
void ctrl_passo(ctrl_estado_t *e, const int16_t *x, const int16_t *y, int n, uint32_t agora,
                int no_menu, int pergunta, ctrl_saida_t *s);
