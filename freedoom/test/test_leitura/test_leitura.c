// Testes da leitura do WAD (w_file_stdc.c) quando o arquivo falha no meio.
//
// No launcher o WAD vem do cartão. Uma falha passageira no cartão fazia o
// fread voltar com menos bytes, e o motor parava na hora com
// "W_ReadLump: only read 1184 of 2792". Agora ele tenta de novo.
//
//   pio test -e native   (no Deck: dentro do distrobox dev-gcc)
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unity.h>

#include "doomtype.h"
#include "w_file.h"

// O motor guarda o FILE logo depois do wad_file_t (w_file_stdc.c).
typedef struct
{
    wad_file_t wad;
    FILE *fstream;
} stdc_wad_file_t;

size_t W_StdC_Read(wad_file_t *wad, unsigned int offset, void *buffer, size_t buffer_len);

// O que o motor usa e não interessa aqui.
void *Z_Malloc(int size, int tag, void *user) { (void) size; (void) tag; (void) user; return NULL; }
void Z_Free(void *ptr) { (void) ptr; }
long M_FileLength(FILE *f) { (void) f; return 0; }
void I_Sleep(int ms) { (void) ms; }

// ---- buffer de leitura da placa (RAM interna com DMA, ver DG_BufferLeitura) ----
//
// Na placa, ler 512 bytes ou mais do cartão direto num destino na PSRAM obriga o
// driver a pegar um buffer de DMA na RAM interna; com ela esgotada a leitura falha
// ("allocate_dma_buf: not enough mem") e não adianta tentar de novo. O buffer da
// placa é reservado quando o jogo abre e o motor lê por ele.
#define BUFFER_DMA 1024
static unsigned char buffer_dma[BUFFER_DMA] __attribute__((aligned(4)));
static int exigir_dma;  // 1 = o "cartão" só aceita ler no buffer de DMA, alinhado

byte *DG_BufferLeitura(int *tamanho)
{
    *tamanho = BUFFER_DMA;
    return buffer_dma;
}

static int destino_aceito(const void *buf, size_t n)
{
    const unsigned char *b = (const unsigned char *) buf;
    if (!exigir_dma || n < 512) return 1;
    return b >= buffer_dma && b + n <= buffer_dma + BUFFER_DMA && (b - buffer_dma) % 4 == 0;
}

// O fread da placa leva o destino até o driver do cartão (o da glibc no PC usa um
// buffer próprio), então a regra do DMA fica aqui: -Wl,--wrap=fread no platformio.ini.
size_t __real_fread(void *ptr, size_t size, size_t n, FILE *f);
size_t __wrap_fread(void *ptr, size_t size, size_t n, FILE *f)
{
    if (!destino_aceito(ptr, size * n)) return 0;  // "allocate_dma_buf: not enough mem"
    return __real_fread(ptr, size, n, f);
}

// ---- arquivo falso: 4 KB com bytes conhecidos e falhas programadas ----

#define TAMANHO 4096
static unsigned char conteudo[TAMANHO];

typedef struct
{
    off_t pos;
    int falhas;      // quantas leituras ainda vão falhar
    off_t falha_em;  // a leitura que cruzar este ponto para nele e falha
} falso_t;

static ssize_t falso_ler(void *c, char *buf, size_t n)
{
    falso_t *f = c;
    if (f->pos >= TAMANHO) return 0;
    if (n > (size_t) (TAMANHO - f->pos)) n = TAMANHO - f->pos;
    if (f->falhas > 0 && f->pos <= f->falha_em && f->pos + (off_t) n > f->falha_em)
    {
        if (f->pos < f->falha_em)
        {
            n = f->falha_em - f->pos;  // entrega até o ponto da falha...
        }
        else
        {
            f->falhas--;
            return -1;  // ...e então dá erro, como o cartão
        }
    }
    memcpy(buf, conteudo + f->pos, n);
    f->pos += n;
    return n;
}

static int falso_posicionar(void *c, off64_t *pos, int de_onde)
{
    falso_t *f = c;
    off_t novo = de_onde == SEEK_SET ? *pos : (de_onde == SEEK_CUR ? f->pos + *pos : TAMANHO + *pos);
    if (novo < 0) return -1;
    f->pos = novo;
    *pos = novo;
    return 0;
}

static falso_t falso;
static stdc_wad_file_t wad;

static void abrir(int falhas, off_t falha_em)
{
    cookie_io_functions_t io = {falso_ler, NULL, falso_posicionar, NULL};
    falso.pos = 0;
    falso.falhas = falhas;
    falso.falha_em = falha_em;
    wad.fstream = fopencookie(&falso, "rb", io);
    setvbuf(wad.fstream, NULL, _IONBF, 0);  // cada fread chega no "cartão"
    wad.wad.mapped = NULL;
    wad.wad.length = TAMANHO;
}

void setUp(void)
{
    exigir_dma = 0;
    for (int i = 0; i < TAMANHO; i++) conteudo[i] = (unsigned char) (i * 7 + 3);
}

void tearDown(void)
{
    fclose(wad.fstream);
}

// ---- testes ----

void test_falha_passageira_no_meio_le_tudo_mesmo_assim(void)
{
    unsigned char buf[2792];
    abrir(1, 1000 + 1184);  // como na placa: 1184 de 2792 e erro
    size_t lido = W_StdC_Read(&wad.wad, 1000, buf, sizeof(buf));
    TEST_ASSERT_EQUAL(sizeof(buf), lido);
    TEST_ASSERT_EQUAL_MEMORY(conteudo + 1000, buf, sizeof(buf));
}

void test_leitura_sem_falha_continua_igual(void)
{
    unsigned char buf[500];
    abrir(0, 0);
    TEST_ASSERT_EQUAL(500, W_StdC_Read(&wad.wad, 3000, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_MEMORY(conteudo + 3000, buf, sizeof(buf));
}

void test_pedir_alem_do_fim_devolve_so_o_que_existe(void)
{
    unsigned char buf[1000];
    abrir(0, 0);
    TEST_ASSERT_EQUAL(TAMANHO - 3500, W_StdC_Read(&wad.wad, 3500, buf, sizeof(buf)));
}

void test_falha_que_nao_passa_desiste_sem_travar(void)
{
    unsigned char buf[1000];
    abrir(1000000, 200);  // o cartão nunca volta
    size_t lido = W_StdC_Read(&wad.wad, 0, buf, sizeof(buf));
    TEST_ASSERT_EQUAL(200, lido);  // o motor então mostra o erro, como antes
}

// Como na placa em 14/11/2026: 1184 de 2792 bytes e depois erro, sempre.
void test_destino_sem_dma_le_pelo_buffer_da_placa(void)
{
    static unsigned char buf[2792];
    abrir(0, 0);
    exigir_dma = 1;
    size_t lido = W_StdC_Read(&wad.wad, 352, buf, sizeof(buf));  // 352: fora do início de um setor
    TEST_ASSERT_EQUAL(sizeof(buf), lido);
    TEST_ASSERT_EQUAL_MEMORY(conteudo + 352, buf, sizeof(buf));
}

void test_buffer_da_placa_ate_o_fim_do_arquivo(void)
{
    static unsigned char buf[1500];
    abrir(0, 0);
    exigir_dma = 1;
    TEST_ASSERT_EQUAL(TAMANHO - 3000, W_StdC_Read(&wad.wad, 3000, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_MEMORY(conteudo + 3000, buf, TAMANHO - 3000);
}

void test_buffer_da_placa_com_falha_passageira(void)
{
    static unsigned char buf[2792];
    abrir(1, 1500);
    exigir_dma = 1;
    TEST_ASSERT_EQUAL(sizeof(buf), W_StdC_Read(&wad.wad, 700, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_MEMORY(conteudo + 700, buf, sizeof(buf));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_falha_passageira_no_meio_le_tudo_mesmo_assim);
    RUN_TEST(test_leitura_sem_falha_continua_igual);
    RUN_TEST(test_pedir_alem_do_fim_devolve_so_o_que_existe);
    RUN_TEST(test_falha_que_nao_passa_desiste_sem_travar);
    RUN_TEST(test_destino_sem_dma_le_pelo_buffer_da_placa);
    RUN_TEST(test_buffer_da_placa_ate_o_fim_do_arquivo);
    RUN_TEST(test_buffer_da_placa_com_falha_passageira);
    return UNITY_END();
}
