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

size_t W_StdC_Read(wad_file_t *wad, unsigned int offset,
                   void *buffer, size_t buffer_len)
{
    stdc_wad_file_t *stdc_wad;
    size_t result = 0;
    int tentativa;

    stdc_wad = (stdc_wad_file_t *) wad;

    // [JC3248W535] No launcher o WAD vem do cartão, que às vezes falha uma
    // leitura no meio (fread volta curto com erro). Tenta de novo a partir de
    // onde parou; só o fim do arquivo encerra sem erro.
    for (tentativa = 0; tentativa < 5 && result < buffer_len; tentativa++)
    {
        size_t lido;

        if (tentativa > 0)
        {
            printf("W_Read: leitura incompleta em %u (%u de %u bytes), tentando de novo\n",
                   offset, (unsigned) result, (unsigned) buffer_len);
            clearerr(stdc_wad->fstream);
            I_Sleep(20 * tentativa);
        }

        // Jump to the specified position in the file.

        fseek(stdc_wad->fstream, offset + result, SEEK_SET);

        // Read into the buffer.

        lido = fread((byte *) buffer + result, 1, buffer_len - result, stdc_wad->fstream);
        result += lido;

        if (feof(stdc_wad->fstream))
        {
            break;  // pediu além do fim: não adianta tentar
        }
    }

    return result;
}


wad_file_class_t stdc_wad_file = 
{
    W_StdC_OpenFile,
    W_StdC_CloseFile,
    W_StdC_Read,
};


