// Vídeo do Doom na JC3248W535 (igual no firmware próprio e no launcher).
//
// O Doom desenha 320x200 com 256 cores em I_VideoBuffer. Aqui a imagem é
// esticada para 480x320 e convertida para RGB565 na memória da tela
// (placa_fb). Uma tarefa no núcleo 0 converte e manda para a tela enquanto o
// jogo já calcula o quadro seguinte no núcleo 1.
//
// A forma dessa memória vem do config_placa.h de cada firmware:
//   FB_LARGURA x FB_ALTURA  tamanho da imagem
//   FB_GIRO     0 = deitada (480x320, quem gira é o launcher);
//               1 ou 3 = em pé, como o painel (320x480), girada aqui
//   FB_TROCA_BYTES  1 = RGB565 com os dois bytes trocados (LV_COLOR_16_SWAP)
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"

#include "config.h"
#include "d_event.h"
#include "d_main.h"
#include "doomgeneric.h"
#include "doomstat.h"
#include "i_system.h"
#include "i_video.h"
#include "m_argv.h"
#include "v_video.h"
#include "z_zone.h"

#include "config_placa.h"
#include "placa.h"

byte *I_VideoBuffer = NULL;
boolean screensaver_mode = false;
boolean screenvisible;
float mouse_acceleration = 2.0;
int mouse_threshold = 10;
int usegamma = 0;
int usemouse = 1;  // arrastar no botão ATIRAR gira como um mouse

extern boolean menuactive;

static uint16_t paleta[256];        // cores atuais em RGB565
static uint16_t paleta_copia[256];  // as do quadro que está indo para a tela
static byte *copia;                 // quadro que está indo para a tela
static int modo_copia;

#if FB_GIRO == 0
// Para cada coluna da imagem: coluna do Doom. Para cada linha: início da
// linha do Doom (linha * 320).
static uint16_t origem_x[FB_LARGURA];
static uint16_t origem_y[FB_ALTURA];
#else
// Para cada linha do painel (0..479): coluna do Doom (0..319).
static uint16_t coluna_origem[FB_ALTURA];
// Para cada coluna do painel (0..319): início da linha do Doom (linha * 320).
static uint16_t linha_origem[FB_LARGURA];
#endif

static SemaphoreHandle_t sem_livre;   // a tarefa da tela terminou
static SemaphoreHandle_t sem_pronto;  // há um quadro novo em `copia`
static TaskHandle_t tarefa = NULL;

static void montar_tabelas(void)
{
    int px, py;

#if FB_GIRO == 0
    for (px = 0; px < FB_LARGURA; px++)
    {
        origem_x[px] = px * SCREENWIDTH / FB_LARGURA;
    }
    for (py = 0; py < FB_ALTURA; py++)
    {
        origem_y[py] = (py * SCREENHEIGHT / FB_ALTURA) * SCREENWIDTH;
    }
#else
    for (py = 0; py < FB_ALTURA; py++)
    {
#if FB_GIRO == 1
        int lx = py;
#else
        int lx = FB_ALTURA - 1 - py;
#endif
        coluna_origem[py] = lx * SCREENWIDTH / FB_ALTURA;
    }
    for (px = 0; px < FB_LARGURA; px++)
    {
#if FB_GIRO == 1
        int ly = FB_LARGURA - 1 - px;
#else
        int ly = px;
#endif
        linha_origem[px] = (ly * SCREENHEIGHT / FB_LARGURA) * SCREENWIDTH;
    }
#endif
}

static void converter(void)
{
    uint16_t *saida = placa_fb();
    int py, px;

    for (py = 0; py < FB_ALTURA; py++, saida += FB_LARGURA)
    {
        const byte *origem;

#if FB_GIRO == 0
        // 320 linhas da imagem vêm de 200 linhas do Doom: as repetidas são
        // copiadas da de cima.
        if (py > 0 && origem_y[py] == origem_y[py - 1])
        {
            memcpy(saida, saida - FB_LARGURA, FB_LARGURA * 2);
            continue;
        }
        origem = copia + origem_y[py];
        for (px = 0; px < FB_LARGURA; px++)
        {
            saida[px] = paleta_copia[origem[origem_x[px]]];
        }
#else
        // 480 linhas do painel vêm de 320 colunas do Doom: uma em cada três
        // é repetida.
        if (py > 0 && coluna_origem[py] == coluna_origem[py - 1])
        {
            memcpy(saida, saida - FB_LARGURA, FB_LARGURA * 2);
            continue;
        }
        origem = copia + coluna_origem[py];
        for (px = 0; px < FB_LARGURA; px++)
        {
            saida[px] = paleta_copia[origem[linha_origem[px]]];
        }
#endif
    }
}

static void tarefa_tela(void *arg)
{
    (void) arg;

    for (;;)
    {
        xSemaphoreTake(sem_pronto, portMAX_DELAY);
        converter();
        placa_desenhar_botoes(modo_copia);
        placa_enviar();
        xSemaphoreGive(sem_livre);
    }
}

void I_InitGraphics(void)
{
    montar_tabelas();

    // A imagem do Doom fica na RAM interna (mais rápida) se sobrar bastante:
    // os drivers do cartão e do som também precisam dela.
    if (heap_caps_get_free_size(MALLOC_CAP_INTERNAL) > SCREENWIDTH * SCREENHEIGHT + 48 * 1024)
    {
        I_VideoBuffer = heap_caps_malloc(SCREENWIDTH * SCREENHEIGHT,
                                         MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    if (I_VideoBuffer == NULL)
    {
        I_VideoBuffer = malloc(SCREENWIDTH * SCREENHEIGHT);
    }
    copia = malloc(SCREENWIDTH * SCREENHEIGHT);
    if (I_VideoBuffer == NULL || copia == NULL)
    {
        I_Error("Sem memoria para a imagem do jogo");
    }
    memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT);

    sem_livre = xSemaphoreCreateBinary();
    sem_pronto = xSemaphoreCreateBinary();
    xSemaphoreGive(sem_livre);

    placa_console_fim();
    xTaskCreatePinnedToCore(tarefa_tela, "tela", 4096, NULL, 2, &tarefa, 0);

    screenvisible = true;

    extern void I_InitInput(void);
    I_InitInput();
}

void I_ShutdownGraphics(void)
{
}

void I_StartFrame(void)
{
}

void I_StartTic(void)
{
    extern void I_GetEvent(void);
    I_GetEvent();
}

void I_UpdateNoBlit(void)
{
}

void I_FinishUpdate(void)
{
    // Espera a tela terminar o quadro anterior e entrega o novo.
    xSemaphoreTake(sem_livre, portMAX_DELAY);
    memcpy(copia, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
    memcpy(paleta_copia, paleta, sizeof(paleta));
    modo_copia = (!menuactive && gamestate == GS_LEVEL && !demoplayback) ? 1 : 0;
    xSemaphoreGive(sem_pronto);
}

void I_ReadScreen(byte *scr)
{
    memcpy(scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

void I_SetPalette(byte *palette)
{
    int i;

    for (i = 0; i < 256; ++i)
    {
        int r = gammatable[usegamma][*palette++];
        int g = gammatable[usegamma][*palette++];
        int b = gammatable[usegamma][*palette++];

        uint16_t c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

#if FB_TROCA_BYTES
        c = (uint16_t) ((c << 8) | (c >> 8));
#endif
        paleta[i] = c;
    }
}

int I_GetPaletteIndex(int r, int g, int b)
{
    int best = 0, best_diff = 0x7FFFFFFF, i;

    for (i = 0; i < 256; ++i)
    {
        uint16_t c = paleta[i];
#if FB_TROCA_BYTES
        c = (uint16_t) ((c << 8) | (c >> 8));
#endif
        int pr = (c >> 8) & 0xF8;
        int pg = (c >> 3) & 0xFC;
        int pb = (c << 3) & 0xF8;
        int diff = (r - pr) * (r - pr) + (g - pg) * (g - pg) + (b - pb) * (b - pb);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }
    }
    return best;
}

// Erro fatal do motor (I_Error): mostra na tela e para aqui.
void DG_ErroFatal(const char *msg)
{
    printf("\nERRO: %s\n", msg);
    fflush(stdout);
    if (tarefa != NULL)
    {
        // Espera a tarefa da tela largar a imagem; ela não roda mais depois.
        xSemaphoreTake(sem_livre, pdMS_TO_TICKS(1000));
    }
    placa_tela_erro("ERRO", msg);
    placa_enviar();
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// "Sair do jogo" no menu: reinicia a placa.
void DG_Sair(void)
{
    placa_reiniciar();
}

void DG_Init(void)
{
}

void DG_DrawFrame(void)
{
}

void DG_SetWindowTitle(const char *title)
{
    (void) title;
}

void DG_SleepMs(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms) ? pdMS_TO_TICKS(ms) : 1);
}

uint32_t DG_GetTicksMs(void)
{
    return placa_millis();
}

void I_BeginRead(void) {}
void I_EndRead(void) {}
void I_SetWindowTitle(char *title) { (void) title; }
void I_GraphicsCheckCommandLine(void) {}
void I_SetGrabMouseCallback(grabmouse_callback_t func) { (void) func; }
void I_EnableLoadingDisk(void) {}
void I_BindVideoVariables(void) {}
void I_DisplayFPSDots(boolean dots_on) { (void) dots_on; }
void I_CheckIsScreensaver(void) {}
