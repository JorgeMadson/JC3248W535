#include <stdio.h>

#include "m_argv.h"

#include "doomgeneric.h"

pixel_t* DG_ScreenBuffer = NULL;

void M_FindResponseFile(void);
void D_DoomMain (void);


void doomgeneric_Create(int argc, char **argv)
{
	// save arguments
    myargc = argc;
    myargv = argv;

	M_FindResponseFile();

	// [JC3248W535] a tela e montada em src/i_video.c (sem DG_ScreenBuffer);
	// as tabelas grandes do motor vao para a PSRAM.
	{
	    void DG_AlocarTicdata(void);
	    void DG_AlocarPlanos(void);
	    void DG_AlocarEstatisticas(void);
	    DG_AlocarTicdata();
	    DG_AlocarPlanos();
	    DG_AlocarEstatisticas();
	}

	DG_Init();

	D_DoomMain ();
}

