# doomgeneric (motor do Doom)

Cópia de <https://github.com/ozkl/doomgeneric>, commit
`e257a09527062bfb986b6e5e0780839b6cbd5b71` (11/10/2026). Licença GPL v2 (`LICENSE`).

Só entraram os arquivos da lista `SRC_DOOM` do Makefile original. Ficaram de fora
os `doomgeneric_*.c` de outras plataformas, o som/música SDL e Allegro e o
`i_video.c` (o nosso está em `freedoom/src/i_video.c`).

## O que foi alterado

Cada mudança está marcada com `[JC3248W535]` no código (`grep -rn JC3248W535 src`).

| Arquivo | Mudança |
|---|---|
| `i_sound.c` | Sem SDL_mixer; usa o `DG_sound_module` de `src/i_som.c`; sem música |
| `i_system.c` | Zona de memória ajustável; `I_Quit` reinicia a placa; `I_Error` mostra o erro na tela |
| `w_file.c` | Tenta primeiro o WAD da partição `wad` (`src/w_file_flash.c`) |
| `d_iwad.c` | Aceita o caminho `/flash/...` sem procurar arquivo |
| `g_game.c` | `G_ProximaArma()` para o botão ARMA do toque |
| `m_config.c` | Pasta de configuração e saves vem de `DG_PastaDados()` |
| `m_menu.c` | Save novo já nasce com o nome do mapa (não há teclado) |
