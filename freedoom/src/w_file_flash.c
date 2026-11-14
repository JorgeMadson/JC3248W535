// WAD guardado na partição `wad` da flash (ver particoes.csv).
//
// A partição é mapeada na memória, então o motor usa os dados direto da flash
// sem copiar para a RAM (W_CacheLumpNum devolve ponteiro para dentro do mapa).
// Se o mapeamento falhar, cai para leitura normal com esp_partition_read.
#include <stdio.h>
#include <string.h>

#include "esp_partition.h"

#include "doomtype.h"
#include "w_file.h"
#include "z_zone.h"

typedef struct
{
    wad_file_t wad;
    const esp_partition_t *particao;
    spi_flash_mmap_handle_t mapa;
} flash_wad_file_t;

extern wad_file_class_t flash_wad_file;

static wad_file_t *W_Flash_OpenFile(char *path)
{
    const esp_partition_t *p;
    flash_wad_file_t *result;
    const void *ptr = NULL;
    spi_flash_mmap_handle_t mapa = 0;
    struct { char id[4]; int numlumps; int infotableofs; } cab;
    unsigned int tamanho;

    if (strncmp(path, "/flash/", 7) != 0)
    {
        return NULL;
    }

    p = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                 ESP_PARTITION_SUBTYPE_ANY, "wad");
    if (p == NULL || esp_partition_read(p, 0, &cab, sizeof(cab)) != ESP_OK)
    {
        return NULL;
    }
    if (memcmp(cab.id, "IWAD", 4) != 0 && memcmp(cab.id, "PWAD", 4) != 0)
    {
        return NULL;
    }

    // O diretório fica no fim do arquivo: fim dele = tamanho do WAD.
    tamanho = (unsigned int) cab.infotableofs + (unsigned int) cab.numlumps * 16;
    if (tamanho > p->size)
    {
        printf("WAD da flash maior que a particao (%u > %u)\n",
               tamanho, (unsigned int) p->size);
        return NULL;
    }

    if (esp_partition_mmap(p, 0, tamanho, SPI_FLASH_MMAP_DATA, &ptr, &mapa) != ESP_OK)
    {
        printf("Nao deu para mapear o WAD da flash; lendo por blocos.\n");
        ptr = NULL;
    }

    result = Z_Malloc(sizeof(flash_wad_file_t), PU_STATIC, 0);
    result->wad.file_class = &flash_wad_file;
    result->wad.mapped = (byte *) ptr;
    result->wad.length = tamanho;
    result->particao = p;
    result->mapa = mapa;

    printf("WAD da flash: %u bytes%s\n", tamanho, ptr ? ", mapeado" : "");

    return &result->wad;
}

static void W_Flash_CloseFile(wad_file_t *wad)
{
    flash_wad_file_t *f = (flash_wad_file_t *) wad;

    if (f->wad.mapped != NULL)
    {
        spi_flash_munmap(f->mapa);
    }
    Z_Free(f);
}

static size_t W_Flash_Read(wad_file_t *wad, unsigned int offset,
                           void *buffer, size_t buffer_len)
{
    flash_wad_file_t *f = (flash_wad_file_t *) wad;

    if (offset >= f->wad.length)
    {
        return 0;
    }
    if (offset + buffer_len > f->wad.length)
    {
        buffer_len = f->wad.length - offset;
    }
    if (f->wad.mapped != NULL)
    {
        memcpy(buffer, f->wad.mapped + offset, buffer_len);
        return buffer_len;
    }
    if (esp_partition_read(f->particao, offset, buffer, buffer_len) != ESP_OK)
    {
        return 0;
    }
    return buffer_len;
}

wad_file_class_t flash_wad_file =
{
    W_Flash_OpenFile,
    W_Flash_CloseFile,
    W_Flash_Read,
};
