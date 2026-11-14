// Efeitos sonoros do Doom pelo amplificador I2S da placa (conector P6).
//
// Os sons do WAD são PCM de 8 bits, mono, quase todos a 11025 Hz. Uma tarefa
// mistura até 8 canais e entrega em 16 bits estéreo para placa_som_escrever
// (cada firmware liga isso ao I2S do seu jeito). Não há música.
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "deh_str.h"
#include "doomtype.h"
#include "i_sound.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

#include "config_placa.h"
#include "placa.h"

#define CANAIS 8
#define BLOCO 256  // amostras misturadas por vez (~12 ms)

typedef struct
{
    const byte *dados;   // NULL = canal parado
    uint32_t tamanho;    // em amostras
    uint32_t pos;        // amostra atual
    uint32_t fracao;     // parte fracionária da posição (16 bits)
    uint32_t passo;      // avanço por amostra de saída, 16.16
    int volume;          // 0..127
} canal_t;

static canal_t canais[CANAIS];
static SemaphoreHandle_t trava;
static boolean usar_prefixo;
static boolean ligado;

static void tarefa_som(void *arg)
{
    static int32_t mistura[BLOCO];
    static int16_t saida[BLOCO * 2];
    int c, i;

    (void) arg;

    for (;;)
    {
        memset(mistura, 0, sizeof(mistura));

        xSemaphoreTake(trava, portMAX_DELAY);
        for (c = 0; c < CANAIS; c++)
        {
            canal_t *k = &canais[c];

            if (k->dados == NULL)
            {
                continue;
            }
            for (i = 0; i < BLOCO; i++)
            {
                if (k->pos >= k->tamanho)
                {
                    k->dados = NULL;
                    break;
                }
                mistura[i] += ((int) k->dados[k->pos] - 128) * k->volume;
                k->fracao += k->passo;
                k->pos += k->fracao >> 16;
                k->fracao &= 0xFFFF;
            }
        }
        xSemaphoreGive(trava);

        for (i = 0; i < BLOCO; i++)
        {
            // amostra (±128) x volume (127) ~ 14 bits; x2 para usar os 16.
            int32_t v = (mistura[i] * 2 * SOM_GANHO) >> 8;

            if (v > 32767) v = 32767;
            if (v < -32768) v = -32768;
            saida[i * 2] = saida[i * 2 + 1] = (int16_t) v;
        }
        // Bloqueia até caber: é isso que marca o ritmo da tarefa.
        placa_som_escrever(saida, BLOCO);
    }
}

static boolean Som_Init(boolean use_sfx_prefix)
{
    usar_prefixo = use_sfx_prefix;

    if (!placa_som_iniciar(SOM_TAXA_HZ))
    {
        printf("Som: saida indisponivel\n");
        return false;
    }

    trava = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(tarefa_som, "som", 4096, NULL, 5, NULL, 0);
    ligado = true;
    printf("Som: %d Hz\n", SOM_TAXA_HZ);
    return true;
}

static void Som_Shutdown(void)
{
}

static int Som_GetSfxLumpNum(sfxinfo_t *sfx)
{
    char nome[16];

    if (sfx->link != NULL)
    {
        sfx = sfx->link;
    }
    if (usar_prefixo)
    {
        M_snprintf(nome, sizeof(nome), "ds%s", DEH_String(sfx->name));
    }
    else
    {
        M_StringCopy(nome, DEH_String(sfx->name), sizeof(nome));
    }
    // -1 se o WAD não tem esse som: StartSound só ignora.
    return W_CheckNumForName(nome);
}

static void Som_Update(void)
{
}

static void Som_UpdateSoundParams(int channel, int vol, int sep)
{
    (void) sep;  // alto-falante mono

    if (!ligado || channel < 0 || channel >= CANAIS)
    {
        return;
    }
    canais[channel].volume = vol;
}

static int Som_StartSound(sfxinfo_t *sfx, int channel, int vol, int sep)
{
    const byte *lump;
    unsigned int tam_lump, taxa, tamanho;

    (void) sep;

    if (!ligado || channel < 0 || channel >= CANAIS || sfx->lumpnum < 0)
    {
        return -1;
    }

    // Com o WAD na flash isso é só um ponteiro; com o WAD no cartão o som é
    // carregado uma vez e fica na memória.
    tam_lump = W_LumpLength(sfx->lumpnum);
    lump = W_CacheLumpNum(sfx->lumpnum, PU_STATIC);

    // Formato DMX: 03 00, taxa (2 bytes), tamanho (4 bytes), amostras.
    if (tam_lump < 8 || lump[0] != 0x03 || lump[1] != 0x00)
    {
        return -1;
    }
    taxa = (lump[3] << 8) | lump[2];
    tamanho = (lump[7] << 24) | (lump[6] << 16) | (lump[5] << 8) | lump[4];
    if (tamanho > tam_lump - 8 || tamanho <= 48 || taxa == 0)
    {
        return -1;
    }

    xSemaphoreTake(trava, portMAX_DELAY);
    // As 16 primeiras e 16 últimas amostras são enchimento.
    canais[channel].dados = lump + 8 + 16;
    canais[channel].tamanho = tamanho - 32;
    canais[channel].pos = 0;
    canais[channel].fracao = 0;
    canais[channel].passo = (uint32_t) (((uint64_t) taxa << 16) / SOM_TAXA_HZ);
    canais[channel].volume = vol;
    xSemaphoreGive(trava);

    return channel;
}

static void Som_StopSound(int channel)
{
    if (!ligado || channel < 0 || channel >= CANAIS)
    {
        return;
    }
    xSemaphoreTake(trava, portMAX_DELAY);
    canais[channel].dados = NULL;
    xSemaphoreGive(trava);
}

static boolean Som_SoundIsPlaying(int channel)
{
    if (!ligado || channel < 0 || channel >= CANAIS)
    {
        return false;
    }
    return canais[channel].dados != NULL;
}

static void Som_CacheSounds(sfxinfo_t *sounds, int num_sounds)
{
    (void) sounds;
    (void) num_sounds;
}

static snddevice_t dispositivos[] =
{
    SNDDEVICE_SB, SNDDEVICE_PAS, SNDDEVICE_GUS, SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS, SNDDEVICE_AWE32,
};

sound_module_t DG_sound_module =
{
    dispositivos,
    sizeof(dispositivos) / sizeof(*dispositivos),
    Som_Init,
    Som_Shutdown,
    Som_GetSfxLumpNum,
    Som_Update,
    Som_UpdateSoundParams,
    Som_StartSound,
    Som_StopSound,
    Som_SoundIsPlaying,
    Som_CacheSounds,
};
