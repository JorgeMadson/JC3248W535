// Testes dos controles por toque (controles.c).
//
// Em 14/11/2026 a alavanca de ANDAR não andava. O registro da serial mostrou
// que a tela devolve "nenhum dedo" por 10 a 140 ms enquanto o dedo continua
// encostado; a cada falha a alavanca recentrava onde o dedo estava e o
// deslocamento voltava a zero.
//
//   pio test -e native   (no Deck: dentro do distrobox dev-gcc)
#include <string.h>
#include <unity.h>

#include "doomkeys.h"
#include "../../src/comum/controles.h"
#include "../../src/comum/controles.c"

static ctrl_estado_t e;
static uint32_t agora;
static int segurada[256];   // teclas seguradas, a partir dos eventos
static int apertos[256];    // quantas vezes cada tecla desceu
static int trocas_arma;
static int no_menu;

void setUp(void)
{
    memset(&e, 0, sizeof(e));
    memset(segurada, 0, sizeof(segurada));
    memset(apertos, 0, sizeof(apertos));
    trocas_arma = 0;
    agora = 1000;
    no_menu = 0;
}

void tearDown(void) {}

// Uma leitura de 12 ms. n = 0, 1 ou 2 dedos.
static void ler(int n, int x0, int y0, int x1, int y1)
{
    int16_t x[2] = { (int16_t) x0, (int16_t) x1 };
    int16_t y[2] = { (int16_t) y0, (int16_t) y1 };
    ctrl_saida_t s;
    int i;

    ctrl_passo(&e, x, y, n, agora, no_menu, 0, &s);
    for (i = 0; i < s.n_eventos; i++)
    {
        segurada[s.ev[i].tecla] = s.ev[i].pressionada;
        if (s.ev[i].pressionada) apertos[s.ev[i].tecla]++;
    }
    trocas_arma += s.proxima_arma;
    agora += 12;
}

static void um_dedo(int x, int y) { ler(1, x, y, 0, 0); }
static void nada(void) { ler(0, 0, 0, 0, 0); }

// Como na placa: a cada 3 leituras uma vem vazia, às vezes 3 seguidas (36 ms).
static void falha_da_tela(int passo)
{
    if (passo % 3 == 2) nada();
    if (passo % 7 == 6) { nada(); nada(); nada(); }
}

void test_arrastar_para_cima_anda(void)
{
    int i;

    for (i = 0; i < 20; i++) um_dedo(100, 200 - i * 3);
    TEST_ASSERT_TRUE(segurada[KEY_UPARROW]);
}

void test_arrastar_para_cima_com_falhas_da_tela_anda(void)
{
    int i;

    for (i = 0; i < 20; i++)
    {
        um_dedo(100, 200 - i * 3);
        falha_da_tela(i);
    }
    um_dedo(100, 140);
    TEST_ASSERT_TRUE_MESSAGE(segurada[KEY_UPARROW], "a alavanca recentrou numa falha da tela");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, apertos[KEY_UPARROW], "a tecla de andar soltou e voltou");
}

void test_arrastar_para_a_direita_com_falhas_vira(void)
{
    int i;

    for (i = 0; i < 20; i++)
    {
        um_dedo(60 + i * 3, 200);
        falha_da_tela(i);
    }
    um_dedo(120, 200);
    TEST_ASSERT_TRUE(segurada[KEY_RIGHTARROW]);
    TEST_ASSERT_EQUAL_INT(1, apertos[KEY_RIGHTARROW]);
}

void test_soltar_de_verdade_para_de_andar(void)
{
    int i;

    for (i = 0; i < 20; i++) um_dedo(100, 200 - i * 3);
    for (i = 0; i < 25; i++) nada();  // 300 ms sem dedo
    TEST_ASSERT_FALSE(segurada[KEY_UPARROW]);
}

void test_novo_toque_depois_de_soltar_recentra(void)
{
    int i;

    for (i = 0; i < 20; i++) um_dedo(100, 200 - i * 3);
    for (i = 0; i < 25; i++) nada();
    for (i = 0; i < 5; i++) um_dedo(100, 140);  // encosta parado em outro lugar
    TEST_ASSERT_FALSE(segurada[KEY_UPARROW]);
    TEST_ASSERT_FALSE(segurada[KEY_DOWNARROW]);
}

void test_atirar_segurado_com_falhas_nao_solta(void)
{
    int i;

    for (i = 0; i < 20; i++)
    {
        um_dedo(360, 240);
        falha_da_tela(i);
    }
    TEST_ASSERT_TRUE(segurada[KEY_FIRE]);
    TEST_ASSERT_EQUAL_INT(1, apertos[KEY_FIRE]);
}

void test_arma_com_falhas_troca_uma_vez(void)
{
    int i;

    for (i = 0; i < 15; i++)
    {
        um_dedo(300, 100);
        falha_da_tela(i);
    }
    TEST_ASSERT_EQUAL_INT(1, trocas_arma);
}

void test_andar_e_atirar_juntos(void)
{
    int i;

    for (i = 0; i < 20; i++) ler(2, 100, 200 - i * 3, 360, 240);
    TEST_ASSERT_TRUE(segurada[KEY_UPARROW]);
    TEST_ASSERT_TRUE(segurada[KEY_FIRE]);
}

// ATIRAR só atira: arrastar o dedo dentro dele não gera nada além do tiro.
void test_arrastar_no_atirar_so_atira(void)
{
    int i, outras = 0;

    for (i = 0; i < 20; i++) um_dedo(360 + i * 5, 240);
    for (i = 0; i < 256; i++) if (i != KEY_FIRE) outras += apertos[i];
    TEST_ASSERT_TRUE(segurada[KEY_FIRE]);
    TEST_ASSERT_EQUAL_INT(0, outras);
}

// ATIRAR ficou menor: o canto de baixo, perto da alavanca, não atira mais.
void test_atirar_menor_fora_dele_nao_atira(void)
{
    int i;

    for (i = 0; i < 5; i++) um_dedo(260, 300);
    TEST_ASSERT_FALSE(segurada[KEY_FIRE]);
    for (i = 0; i < 25; i++) nada();
    for (i = 0; i < 5; i++) um_dedo(420, 160);
    TEST_ASSERT_FALSE(segurada[KEY_FIRE]);
    for (i = 0; i < 25; i++) nada();
    for (i = 0; i < 5; i++) um_dedo(410, 250);  // no meio do botão desenhado
    TEST_ASSERT_TRUE(segurada[KEY_FIRE]);
}

// No menu, OK continua sendo toda a direita (não só o ATIRAR pequeno).
void test_menu_ok_em_toda_a_direita(void)
{
    no_menu = 1;
    nada();
    um_dedo(260, 300);
    TEST_ASSERT_EQUAL_INT(1, apertos[KEY_ENTER]);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_arrastar_para_cima_anda);
    RUN_TEST(test_arrastar_para_cima_com_falhas_da_tela_anda);
    RUN_TEST(test_arrastar_para_a_direita_com_falhas_vira);
    RUN_TEST(test_soltar_de_verdade_para_de_andar);
    RUN_TEST(test_novo_toque_depois_de_soltar_recentra);
    RUN_TEST(test_atirar_segurado_com_falhas_nao_solta);
    RUN_TEST(test_arma_com_falhas_troca_uma_vez);
    RUN_TEST(test_andar_e_atirar_juntos);
    RUN_TEST(test_arrastar_no_atirar_so_atira);
    RUN_TEST(test_atirar_menor_fora_dele_nao_atira);
    RUN_TEST(test_menu_ok_em_toda_a_direita);
    return UNITY_END();
}
