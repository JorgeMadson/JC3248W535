// Controles por toque: transforma dedos na tela em teclas do Doom.
//
// Tela deitada, 480x320 (as áreas são as mesmas de placa_desenhar_botoes):
//
//   +------+----------------------------+------+
//   | MENU |                            | MAPA |
//   +------+-------------+-------+------+------+
//   |                    |   ARMA   |   USAR   |
//   |      ANDAR         +----------+----------+
//   |  (alavanca: onde   |                     |
//   |   o dedo encostar  |       ATIRAR        |
//   |   vira o centro)   |  (arrastar = mirar) |
//   +--------------------+---------------------+
//
// No menu: a alavanca move a seleção, o lado direito é OK e MENU volta.
#include <stdlib.h>

#include "d_event.h"
#include "doomgeneric.h"
#include "doomkeys.h"
#include "doomstat.h"
#include "doomtype.h"

#include "placa.h"

extern boolean menuactive;
extern int messageToPrint;
void G_ProximaArma(int direcao);

#define ZONA_MORTA 18     // px até a alavanca começar a valer
#define DIST_CORRER 70    // px para frente/trás em que passa a correr
#define GANHO_MIRA 16     // quanto gira por px arrastado no ATIRAR
#define REPETE_MENU_MS 280
#define INTERVALO_MS 12   // tempo mínimo entre leituras do toque

// Teclas que ficam seguradas enquanto o dedo está lá.
enum { T_FRENTE, T_TRAS, T_ESQ, T_DIR, T_CORRER, T_ATIRAR, T_USAR, T_QTD };
static const unsigned char tecla_de[T_QTD] = {
    KEY_UPARROW, KEY_DOWNARROW, KEY_LEFTARROW, KEY_RIGHTARROW,
    KEY_RSHIFT, KEY_FIRE, KEY_USE,
};
static boolean segurada[T_QTD];

// Fila de eventos para o motor (DG_GetKey tira um por vez).
#define FILA 64
static struct { unsigned char pressionada, tecla; } fila[FILA];
static int fila_ini, fila_fim;

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

static void toque_rapido(unsigned char tecla)
{
    por(1, tecla);
    por(0, tecla);
}

static void segurar(int t, boolean quer)
{
    if (segurada[t] != quer)
    {
        segurada[t] = quer;
        por(quer, tecla_de[t]);
    }
}

// Estado do quadro anterior, para detectar o momento em que o dedo encosta.
static boolean alavanca_ativa;
static int centro_x, centro_y;
static boolean antes_menu, antes_mapa, antes_arma, antes_direita;
static boolean mirando;
static int mira_x;
static int dir_menu;            // última direção mandada no menu (tecla) ou 0
static uint32_t dir_menu_quando;
static boolean modo_menu_antes;

static void atualizar(void)
{
    int16_t x[2], y[2];
    int n = placa_ler_toques(x, y);
    int i;
    boolean no_menu = menuactive;
    boolean tem_alavanca = false, em_menu = false, em_mapa = false;
    boolean em_arma = false, em_usar = false, em_atirar = false;
    int ax = 0, ay = 0, tx = 0;
    boolean quer[T_QTD] = { false };
    uint32_t agora = placa_millis();

    // Ao entrar ou sair do menu, solta tudo para nenhuma tecla ficar presa.
    if (no_menu != modo_menu_antes)
    {
        for (i = 0; i < T_QTD; i++) segurar(i, false);
        modo_menu_antes = no_menu;
        dir_menu = 0;
        mirando = false;
    }

    // Em que área está cada dedo.
    for (i = 0; i < n; i++)
    {
        if (alavanca_ativa && !tem_alavanca && x[i] < 300 && (y[i] >= 64 || x[i] >= 80))
        {
            tem_alavanca = true; ax = x[i]; ay = y[i];
        }
        else if (y[i] < 64 && x[i] < 80) em_menu = true;
        else if (y[i] < 64 && x[i] >= 400) em_mapa = true;
        else if (x[i] < 240 && y[i] >= 64 && !tem_alavanca)
        {
            tem_alavanca = true; ax = x[i]; ay = y[i];
            if (!alavanca_ativa) { centro_x = ax; centro_y = ay; }
        }
        else if (x[i] >= 240 && y[i] >= 64 && y[i] < 150)
        {
            if (x[i] < 360) em_arma = true; else em_usar = true;
        }
        else if (x[i] >= 240 && y[i] >= 150)
        {
            em_atirar = true; tx = x[i];
        }
    }
    alavanca_ativa = tem_alavanca;

    if (em_menu && !antes_menu) toque_rapido(KEY_ESCAPE);
    antes_menu = em_menu;

    if (no_menu)
    {
        boolean direita = em_arma || em_usar || em_atirar;
        int dir = 0;

        if (tem_alavanca)
        {
            int dx = ax - centro_x, dy = ay - centro_y;

            if (abs(dy) >= abs(dx))
            {
                if (dy < -ZONA_MORTA) dir = KEY_UPARROW;
                else if (dy > ZONA_MORTA) dir = KEY_DOWNARROW;
            }
            else
            {
                if (dx < -ZONA_MORTA) dir = KEY_LEFTARROW;
                else if (dx > ZONA_MORTA) dir = KEY_RIGHTARROW;
            }
        }
        if (dir != 0 && (dir != dir_menu || agora - dir_menu_quando > REPETE_MENU_MS))
        {
            toque_rapido(dir);
            dir_menu_quando = agora;
        }
        dir_menu = dir;

        if (direita && !antes_direita)
        {
            // Perguntas de "sim ou não" esperam a letra Y.
            toque_rapido(messageToPrint ? 'y' : KEY_ENTER);
        }
        antes_direita = direita;
        antes_mapa = em_mapa;
        antes_arma = em_arma;
        return;
    }

    // ---- jogando ----
    if (tem_alavanca)
    {
        int dx = ax - centro_x, dy = ay - centro_y;

        quer[T_FRENTE] = dy < -ZONA_MORTA;
        quer[T_TRAS] = dy > ZONA_MORTA;
        quer[T_ESQ] = dx < -ZONA_MORTA;
        quer[T_DIR] = dx > ZONA_MORTA;
        quer[T_CORRER] = abs(dy) > DIST_CORRER;
    }
    quer[T_ATIRAR] = em_atirar;
    quer[T_USAR] = em_usar;
    for (i = 0; i < T_QTD; i++) segurar(i, quer[i]);

    if (em_mapa && !antes_mapa) toque_rapido(KEY_TAB);
    antes_mapa = em_mapa;

    if (em_arma && !antes_arma) G_ProximaArma(1);
    antes_arma = em_arma;

    // Arrastar o dedo no ATIRAR gira o jogador, como um mouse.
    if (em_atirar)
    {
        if (mirando && tx != mira_x && gamestate == GS_LEVEL)
        {
            event_t ev;

            ev.type = ev_mouse;
            ev.data1 = 0;
            ev.data2 = (tx - mira_x) * GANHO_MIRA;
            ev.data3 = 0;
            D_PostEvent(&ev);
        }
        mira_x = tx;
    }
    mirando = em_atirar;
    antes_direita = em_arma || em_usar || em_atirar;
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
