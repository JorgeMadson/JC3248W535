// Controles por toque: transforma dedos na tela em teclas do Doom.
//
// Tela deitada, 480x320 (as áreas são as mesmas de placa_desenhar_botoes):
//
//   +------+----------------------------+------+
//   | MENU |                            | MAPA |
//   +------+-------------+------+-------+------+
//   |                    | ARMA |     USAR     |
//   |      ANDAR         +------+--------------+
//   |  (cima/baixo anda, |                     |
//   |   lados viram;     |        +----------+ |
//   |   longe = correr)  |        |  ATIRAR  | |
//   |                    |        +----------+ |
//   +--------------------+---------------------+
//
// ATIRAR só atira (desenho 120x120 em 350,186; o toque vale a partir de
// x 330, y 166, um pouco além do desenho). O resto da direita não faz nada.
// No menu: a alavanca move a seleção, o lado direito é OK e MENU volta.
#include <stdlib.h>
#include <string.h>

#include "controles.h"
#include "doomkeys.h"

#define ZONA_MORTA 18     // px até a alavanca começar a valer
#define DIST_CORRER 70    // px para frente/trás em que passa a correr
#define ATIRAR_X 330      // canto de cima à esquerda da área de toque do ATIRAR
#define ATIRAR_Y 166
#define REPETE_MENU_MS 280
// A tela às vezes diz "nenhum dedo" por até ~140 ms com o dedo encostado
// (medido em 14/11/2026). Uma área só solta depois desse tempo sem dedo.
#define SOLTAR_MS 150

enum { A_ALAVANCA, A_MENU, A_MAPA, A_ARMA, A_USAR, A_ATIRAR, A_DIREITA };

// Devolve se a área está com dedo, segurando durante as falhas curtas da tela.
static int area(ctrl_estado_t *e, int a, int com_dedo, uint32_t agora)
{
    if (com_dedo)
    {
        e->visto[a] = agora;
        e->ativa[a] = 1;
        return 1;
    }
    if (e->ativa[a] && agora - e->visto[a] < SOLTAR_MS)
    {
        return 1;
    }
    e->ativa[a] = 0;
    return 0;
}

// Teclas que ficam seguradas enquanto o dedo está lá.
enum { T_FRENTE, T_TRAS, T_ESQ, T_DIR, T_CORRER, T_ATIRAR, T_USAR, T_QTD };
static const unsigned char tecla_de[T_QTD] = {
    KEY_UPARROW, KEY_DOWNARROW, KEY_LEFTARROW, KEY_RIGHTARROW,
    KEY_RSHIFT, KEY_FIRE, KEY_USE,
};

static void por(ctrl_saida_t *s, int pressionada, unsigned char tecla)
{
    if (s->n_eventos >= CTRL_MAX_EVENTOS)
    {
        return;
    }
    s->ev[s->n_eventos].pressionada = (unsigned char) pressionada;
    s->ev[s->n_eventos].tecla = tecla;
    s->n_eventos++;
}

static void toque_rapido(ctrl_saida_t *s, unsigned char tecla)
{
    por(s, 1, tecla);
    por(s, 0, tecla);
}

static void segurar(ctrl_estado_t *e, ctrl_saida_t *s, int t, int quer)
{
    quer = quer ? 1 : 0;
    if (e->segurada[t] != quer)
    {
        e->segurada[t] = quer;
        por(s, quer, tecla_de[t]);
    }
}

void ctrl_passo(ctrl_estado_t *e, const int16_t *x, const int16_t *y, int n, uint32_t agora,
                int no_menu, int pergunta, ctrl_saida_t *s)
{
    int i;
    int tem_alavanca = 0, em_menu = 0, em_mapa = 0;
    int em_arma = 0, em_usar = 0, em_atirar = 0, em_direita = 0;
    int ax = 0, ay = 0;
    int quer[T_QTD] = { 0 };

    memset(s, 0, sizeof(*s));
    no_menu = no_menu ? 1 : 0;

    // Ao entrar ou sair do menu, solta tudo para nenhuma tecla ficar presa.
    if (no_menu != e->modo_menu_antes)
    {
        for (i = 0; i < T_QTD; i++) segurar(e, s, i, 0);
        e->modo_menu_antes = no_menu;
        e->dir_menu = 0;
    }

    // Em que área está cada dedo.
    for (i = 0; i < n; i++)
    {
        int alavanca_antes = tem_alavanca;

        if (e->alavanca_ativa && !tem_alavanca && x[i] < 300 && (y[i] >= 64 || x[i] >= 80))
        {
            tem_alavanca = 1; ax = x[i]; ay = y[i];
        }
        else if (y[i] < 64 && x[i] < 80) em_menu = 1;
        else if (y[i] < 64 && x[i] >= 400) em_mapa = 1;
        else if (x[i] < 240 && y[i] >= 64 && !tem_alavanca)
        {
            tem_alavanca = 1; ax = x[i]; ay = y[i];
            if (!e->alavanca_ativa) { e->centro_x = ax; e->centro_y = ay; }
        }
        else if (x[i] >= 240 && y[i] >= 64 && y[i] < 150)
        {
            if (x[i] < 360) em_arma = 1; else em_usar = 1;
        }
        else if (x[i] >= ATIRAR_X && y[i] >= ATIRAR_Y)
        {
            em_atirar = 1;
        }
        // No menu, toda a direita abaixo do MAPA é OK.
        if (x[i] >= 240 && y[i] >= 64 && tem_alavanca == alavanca_antes) em_direita = 1;
    }
    if (tem_alavanca) { e->ultimo_ax = ax; e->ultimo_ay = ay; }
    if (area(e, A_ALAVANCA, tem_alavanca, agora) && !tem_alavanca)
    {
        tem_alavanca = 1; ax = e->ultimo_ax; ay = e->ultimo_ay;
    }
    em_atirar = area(e, A_ATIRAR, em_atirar, agora);
    em_menu = area(e, A_MENU, em_menu, agora);
    em_mapa = area(e, A_MAPA, em_mapa, agora);
    em_arma = area(e, A_ARMA, em_arma, agora);
    em_usar = area(e, A_USAR, em_usar, agora);
    em_direita = area(e, A_DIREITA, em_direita, agora);
    e->alavanca_ativa = tem_alavanca;

    if (em_menu && !e->antes_menu) toque_rapido(s, KEY_ESCAPE);
    e->antes_menu = em_menu;

    if (no_menu)
    {
        int direita = em_direita;
        int dir = 0;

        if (tem_alavanca)
        {
            int dx = ax - e->centro_x, dy = ay - e->centro_y;

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
        if (dir != 0 && (dir != e->dir_menu || agora - e->dir_menu_quando > REPETE_MENU_MS))
        {
            toque_rapido(s, (unsigned char) dir);
            e->dir_menu_quando = agora;
        }
        e->dir_menu = dir;

        if (direita && !e->antes_direita)
        {
            // Perguntas de "sim ou não" esperam a letra Y.
            toque_rapido(s, pergunta ? 'y' : KEY_ENTER);
        }
        e->antes_direita = direita;
        e->antes_mapa = em_mapa;
        e->antes_arma = em_arma;
        return;
    }

    // ---- jogando ----
    if (tem_alavanca)
    {
        int dx = ax - e->centro_x, dy = ay - e->centro_y;

        quer[T_FRENTE] = dy < -ZONA_MORTA;
        quer[T_TRAS] = dy > ZONA_MORTA;
        quer[T_ESQ] = dx < -ZONA_MORTA;
        quer[T_DIR] = dx > ZONA_MORTA;
        quer[T_CORRER] = abs(dy) > DIST_CORRER;
    }
    quer[T_ATIRAR] = em_atirar;
    quer[T_USAR] = em_usar;
    for (i = 0; i < T_QTD; i++) segurar(e, s, i, quer[i]);

    if (em_mapa && !e->antes_mapa) toque_rapido(s, KEY_TAB);
    e->antes_mapa = em_mapa;

    if (em_arma && !e->antes_arma) s->proxima_arma = 1;
    e->antes_arma = em_arma;

    e->antes_direita = em_direita;
}
