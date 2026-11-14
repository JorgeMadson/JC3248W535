// Testes da zona de memória do motor (z_zone.c) com a PSRAM picada.
//
// No launcher o maior bloco livre de PSRAM pode ser menor do que o Doom
// precisa, mesmo havendo memória de sobra em pedaços. O motor pede ao
// firmware mais pedaços com DG_ZonaExtra e usa todos como uma zona só.
//
//   pio test -e native   (no Deck: dentro do distrobox dev-gcc)
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unity.h>

#include "doomtype.h"
#include "z_zone.h"

// ---- o que o motor pede ao sistema (aqui, falso) ----

#define TAMANHO_PEDACO (64 * 1024)
// Os dois pedaços saem da mesma memória com um buraco no meio, como na PSRAM
// de verdade. (Dois arrays separados ficariam colados e a emenda "encostaria"
// por acaso, escondendo erros do Z_CheckHeap.)
#define BURACO 4096
static unsigned char memoria[2 * TAMANHO_PEDACO + BURACO] __attribute__((aligned(16)));
static unsigned char *const pedaco1 = memoria;
static unsigned char *const pedaco2 = memoria + TAMANHO_PEDACO + BURACO;
static int pedacos_extras;  // quantos pedaços extras ainda dá para entregar

byte *I_ZoneBase(int *size)
{
    *size = TAMANHO_PEDACO;
    return pedaco1;
}

byte *DG_ZonaExtra(int *size)
{
    if (pedacos_extras <= 0)
    {
        return NULL;
    }
    pedacos_extras--;
    *size = TAMANHO_PEDACO;
    return pedaco2;
}

static jmp_buf saida_erro;
static char mensagem_erro[256];

void I_Error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(mensagem_erro, sizeof(mensagem_erro), fmt, ap);
    va_end(ap);
    longjmp(saida_erro, 1);
}

static int dentro(void *p, unsigned char *pedaco)
{
    return (unsigned char *) p >= pedaco && (unsigned char *) p < pedaco + TAMANHO_PEDACO;
}

void setUp(void)
{
    pedacos_extras = 1;
    mensagem_erro[0] = '\0';
}

void tearDown(void) {}

// ---- testes ----

void test_usa_os_dois_pedacos(void)
{
    void *p[10];
    int no1 = 0, no2 = 0;

    if (setjmp(saida_erro))
    {
        TEST_FAIL_MESSAGE(mensagem_erro);
    }
    Z_Init();
    // 100 KB no total: não cabe num pedaço de 64 KB sozinho.
    for (int i = 0; i < 10; i++)
    {
        p[i] = Z_Malloc(10 * 1024, PU_STATIC, NULL);
        memset(p[i], i, 10 * 1024);
        no1 += dentro(p[i], pedaco1);
        no2 += dentro(p[i], pedaco2);
    }
    TEST_ASSERT_EQUAL(10, no1 + no2);
    TEST_ASSERT_TRUE(no1 > 0);
    TEST_ASSERT_TRUE(no2 > 0);
    for (int i = 0; i < 10; i++)
    {
        TEST_ASSERT_EQUAL_HEX8(i, ((unsigned char *) p[i])[10 * 1024 - 1]);  // nada se sobrepôs
    }
    Z_CheckHeap();  // roda a cada fase carregada: tem que aceitar a emenda
}

void test_bloco_nunca_atravessa_a_emenda(void)
{
    if (setjmp(saida_erro))
    {
        // esperado: 100 KB seguidos não cabem em nenhum pedaço
        TEST_ASSERT_NOT_NULL(strstr(mensagem_erro, "Z_Malloc"));
        return;
    }
    Z_Init();
    void *a = Z_Malloc(40 * 1024, PU_STATIC, NULL);
    void *b = Z_Malloc(40 * 1024, PU_STATIC, NULL);
    Z_Free(a);
    Z_Free(b);
    Z_CheckHeap();
    Z_Malloc(100 * 1024, PU_STATIC, NULL);
    TEST_FAIL_MESSAGE("juntou memória de dois pedaços num bloco só");
}

void test_cache_purgavel_e_reaproveitado_nos_dois_pedacos(void)
{
    void *cache[12];

    if (setjmp(saida_erro))
    {
        TEST_FAIL_MESSAGE(mensagem_erro);
    }
    Z_Init();
    // enche os dois pedaços de cache (como os lumps do WAD lidos do cartão)
    for (int i = 0; i < 12; i++)
    {
        cache[i] = NULL;
        Z_Malloc(10 * 1024, PU_CACHE, &cache[i]);
    }
    // um bloco estático grande tem que caber jogando cache fora
    void *grande = Z_Malloc(50 * 1024, PU_STATIC, NULL);
    TEST_ASSERT_NOT_NULL(grande);
    Z_CheckHeap();
}

void test_sem_pedaco_extra_funciona_como_antes(void)
{
    pedacos_extras = 0;
    if (setjmp(saida_erro))
    {
        TEST_FAIL_MESSAGE(mensagem_erro);
    }
    Z_Init();
    void *p = Z_Malloc(60 * 1024, PU_STATIC, NULL);
    TEST_ASSERT_TRUE(dentro(p, pedaco1));
    Z_CheckHeap();
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_usa_os_dois_pedacos);
    RUN_TEST(test_bloco_nunca_atravessa_a_emenda);
    RUN_TEST(test_cache_purgavel_e_reaproveitado_nos_dois_pedacos);
    RUN_TEST(test_sem_pedaco_extra_funciona_como_antes);
    return UNITY_END();
}
