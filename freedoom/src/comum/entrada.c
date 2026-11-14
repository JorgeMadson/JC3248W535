// Liga o toque da placa ao motor do Doom. A lógica (áreas, alavanca, botões)
// fica em controles.c, que não depende do hardware e é testada no PC.
#include <string.h>

#include "d_event.h"
#include "doomgeneric.h"
#include "doomkeys.h"
#include "doomstat.h"
#include "doomtype.h"

#include "controles.h"
#include "placa.h"

extern boolean menuactive;
extern int messageToPrint;
void G_ProximaArma(int direcao);

#define INTERVALO_MS 12   // tempo mínimo entre leituras do toque

// Fila de eventos para o motor (DG_GetKey tira um por vez).
#define FILA 64
static struct { unsigned char pressionada, tecla; } fila[FILA];
static int fila_ini, fila_fim;

static ctrl_estado_t estado;

static void por(int pressionada, unsigned char tecla)
{
    int prox = (fila_fim + 1) % FILA;

    if (prox == fila_ini)
    {
        return;  // cheia: descarta
    }
    fila[fila_fim].pressionada = pressionada;
    fila[fila_fim].tecla = tecla;
    fila_fim = prox;
}

static void atualizar(void)
{
    int16_t x[2], y[2];
    int n = placa_ler_toques(x, y);
    ctrl_saida_t s;
    int i;

    ctrl_passo(&estado, x, y, n, placa_millis(), menuactive, messageToPrint != 0, &s);

    for (i = 0; i < s.n_eventos; i++) por(s.ev[i].pressionada, s.ev[i].tecla);
    if (s.proxima_arma) G_ProximaArma(1);
}

int DG_GetKey(int *pressed, unsigned char *key)
{
    static uint32_t ultima;
    uint32_t agora = placa_millis();

    if (fila_ini == fila_fim && agora - ultima >= INTERVALO_MS)
    {
        ultima = agora;
        atualizar();
    }
    if (fila_ini == fila_fim)
    {
        return 0;
    }
    *pressed = fila[fila_ini].pressionada;
    *key = fila[fila_ini].tecla;
    fila_ini = (fila_ini + 1) % FILA;
    return 1;
}
