#pragma once
// Configuração das regras de cada jogo. Mude aqui, não no meio do código.

#include <stdint.h>

namespace placar {

struct ConfigRiftbound {
  int pontosVitoria = 8;          // Riftbound 1x1: vence a partida quem chega a 8
  int partidasParaSerie = 2;      // melhor de três
  uint32_t tempoRodadaS = 50 * 60;
};

struct ConfigMagic {
  int vidaInicial = 20;
  int opcoesVidaInicial[3] = {20, 30, 40};  // 40 = Commander
  int venenoParaPerder = 10;
  int partidasParaSerie = 2;      // melhor de três
  int vidaMin = -99, vidaMax = 999;
  int venenoMax = 99;
  uint32_t janelaVariacaoMs = 3000;  // "-7" some depois disso sem mexer
  uint32_t tempoRodadaS = 50 * 60;
};

}  // namespace placar
