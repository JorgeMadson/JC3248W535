#include "magic_regras.h"

namespace placar {

static int limitar(int v, int min, int max) { return v < min ? min : (v > max ? max : v); }

MagicRegras::MagicRegras(ConfigMagic cfg) : cfg_(cfg) { zerar(); }

void MagicRegras::mudarVida(int j, int delta, uint32_t agora) {
  if (j < 0 || j > 1 || delta == 0) return;
  int novo = limitar(vida_[j] + delta, cfg_.vidaMin, cfg_.vidaMax);
  int aplicado = novo - vida_[j];
  if (aplicado == 0) return;
  vida_[j] = novo;

  if (variacaoVisivel(j, agora) != 0) variacao_[j] += aplicado;
  else variacao_[j] = aplicado;
  ultimaMudancaMs_[j] = agora;
}

void MagicRegras::mudarVeneno(int j, int delta) {
  if (j < 0 || j > 1) return;
  veneno_[j] = limitar(veneno_[j] + delta, 0, cfg_.venenoMax);
}

void MagicRegras::zerar() {
  for (int j = 0; j < 2; j++) {
    vida_[j] = cfg_.vidaInicial;
    veneno_[j] = 0;
    variacao_[j] = 0;
    ultimaMudancaMs_[j] = 0;
  }
}

void MagicRegras::definirVidaInicial(int vida) {
  cfg_.vidaInicial = limitar(vida, 1, cfg_.vidaMax);
  zerar();
}

int MagicRegras::variacaoVisivel(int j, uint32_t agora) const {
  if (variacao_[j] == 0) return 0;
  if (agora - ultimaMudancaMs_[j] >= cfg_.janelaVariacaoMs) return 0;
  return variacao_[j];
}

}  // namespace placar
