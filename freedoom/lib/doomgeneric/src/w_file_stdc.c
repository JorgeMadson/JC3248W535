//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	WAD I/O functions.
//

#include <stdio.h>
#include <string.h>

#include "m_misc.h"
#include "w_file.h"
#include "i_timer.h"
#include "z_zone.h"

typedef struct
{
    wad_file_t wad;
    FILE *fstream;
} stdc_wad_file_t;

extern wad_file_class_t stdc_wad_file;

static wad_file_t *W_StdC_OpenFile(char *path)
{
    stdc_wad_file_t *result;
    FILE *fstream;

    fstream = fopen(path, "rb");

    if (fstream == NULL)
    {
        return NULL;
    }

    // Create a new stdc_wad_file_t to hold the file handle.

    result = Z_Malloc(sizeof(stdc_wad_file_t), PU_STATIC, 0);
    result->wad.file_class = &stdc_wad_file;
    result->wad.mapped = NULL;
    result->wad.length = M_FileLength(fstream);
    result->fstream = fstream;

    return &result->wad;
}

static void W_StdC_CloseFile(wad_file_t *wad)
{
    stdc_wad_file_t *stdc_wad;

    stdc_wad = (stdc_wad_file_t *) wad;

    fclose(stdc_wad->fstream);
    Z_Free(stdc_wad);
}

// Read data from the specified position in the file into the 
// provided buffer.  Returns the number of bytes read.

// [JC3248W535] Lê buffer_len bytes a partir de offset, tentando de novo se o
// cartão falhar no meio (fread volta curto com erro); só o fim do arquivo
// encerra sem erro.
static size_t LerComTentativas(FILE *f, unsigned int offset, byte *buffer, size_t buffer_len)
{
    size_t result = 0;
    int tentativa;

    for (tentativa = 0; tentativa < 5 && result < buffer_len; tentativa++)
    {
        size_t lido;

        if (tentativa > 0)
        {
            printf("W_Read: leitura incompleta em %u (%u de %u bytes), tentando de novo\n",
                   offset, (unsigned) result, (unsigned) buffer_len);
            clearerr(f);
            I_Sleep(20 * tentativa);
        }

        fseek(f, offset + result, SEEK_SET);
        lido = fread(buffer + result, 1, buffer_len - result, f);
        result += lido;

        if (feof(f))
        {
            break;  // pediu além do fim: não adianta tentar
        }
    }

    return result;
}

// [JC3248W535] Buffer de leitura da placa: RAM interna com DMA, reservada
// quando o jogo abre. Ler do cartão direto num destino na PSRAM obriga o driver
// a pegar um buffer de DMA na RAM interna na hora, e com ela esgotada a leitura
// falha ("allocate_dma_buf: not enough mem"; no jogo, "W_ReadLump: only read
// 1184 of 2792"). Sem buffer da placa (NULL), lê direto como antes.
__attribute__((weak)) byte *DG_BufferLeitura(int *tamanho)
{
    *tamanho = 0;
    return NULL;
}

#define SETOR 512

size_t W_StdC_Read(wad_file_t *wad, unsigned int offset,
                   void *buffer, size_t buffer_len)
{
    stdc_wad_file_t *stdc_wad = (stdc_wad_file_t *) wad;
    byte *destino = (byte *) buffer;
    int tamanho = 0;
    byte *dma = DG_BufferLeitura(&tamanho);
    size_t feito = 0;

    if (dma == NULL || tamanho < SETOR)
    {
        return LerComTentativas(stdc_wad->fstream, offset, destino, buffer_len);
    }
    tamanho -= tamanho % SETOR;

    // Em pedaços que começam no início de um setor: assim o cartão lê os
    // setores inteiros direto no buffer (alinhado) e nada é alocado na hora.
    while (feito < buffer_len)
    {
        unsigned int pos = offset + feito;
        unsigned int inicio = pos - pos % SETOR;
        size_t pular = pos - inicio;
        size_t quer = pular + (buffer_len - feito);
        size_t lido, util;

        if (quer > (size_t) tamanho)
        {
            quer = tamanho;
        }
        lido = LerComTentativas(stdc_wad->fstream, inicio, dma, quer);
        if (lido <= pular)
        {
            break;
        }
        util = lido - pular;
        if (util > buffer_len - feito)
        {
            util = buffer_len - feito;
        }
        memcpy(destino + feito, dma + pular, util);
        feito += util;
        if (lido < quer)
        {
            break;  // fim do arquivo, ou uma falha que não passou
        }
    }

    return feito;
}


wad_file_class_t stdc_wad_file = 
{
    W_StdC_OpenFile,
    W_StdC_CloseFile,
    W_StdC_Read,
};


