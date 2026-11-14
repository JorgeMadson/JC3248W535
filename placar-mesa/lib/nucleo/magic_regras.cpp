#include "magic_regras.h"

namespace placar {

static int limitar(int v, int min, int max) { return v < min ? min : (v > max ? max : v); }

MagicRegras::MagicRegras(ConfigMagic cfg) : cfg_(cfg) { reiniciarVida(); }

void MagicRegras::mudarVida(int j, int delta, uint32_t agora) {
  if (estado_ != Estado::EmJogo || j < 0 || j > 1 || delta == 0) return;
  int antes = vida_[j];
  int novo = limitar(antes + delta, cfg_.vidaMin, cfg_.vidaMax);
  int aplicado = novo - antes;
  if (aplicado == 0) return;
  vida_[j] = novo;

  if (variacaoVisivel(j, agora) != 0) variacao_[j] += aplicado;
  else variacao_[j] = aplicado;
  ultimaMudancaMs_[j] = agora;

  conferirDerrota(j, antes, veneno_[j]);
}

void MagicRegras::mudarVeneno(int j, int delta) {
  if (estado_ != Estado::EmJogo || j < 0 || j > 1) return;
  int antes = veneno_[j];
  veneno_[j] = limitar(antes + delta, 0, cfg_.venenoMax);
  conferirDerrota(j, vida_[j], antes);
}

void MagicRegras::conferirDerrota(int j, int vidaAntes, int venenoAntes) {
  if (!perdeu(j)) return;
  vidaAntes_ = vidaAntes;
  venenoAntes_ = venenoAntes;
  vencedor_ = 1 - j;
  vitorias_[vencedor_]++;
  estado_ = vitorias_[vencedor_] >= cfg_.partidasParaSerie ? Estado::SerieEncerrada : Estado::PartidaEncerrada;
}

void MagicRegras::reiniciarVida() {
  for (int j = 0; j < 2; j++) {
    vida_[j] = cfg_.vidaInicial;
    veneno_[j] = 0;
    variacao_[j] = 0;
    ultimaMudancaMs_[j] = 0;
  }
}

void MagicRegras::zerar() {
  if (estado_ != Estado::EmJogo) return;
  reiniciarVida();
}

void MagicRegras::proximaPartida() {
  if (estado_ != Estado::PartidaEncerrada) return;
  reiniciarVida();
  vencedor_ = -1;
  estado_ = Estado::EmJogo;
}

void MagicRegras::novaSerie() {
  reiniciarVida();
  vitorias_[0] = vitorias_[1] = 0;
  vencedor_ = -1;
  estado_ = Estado::EmJogo;
}

void MagicRegras::desfazerDerrota() {
  if (estado_ == Estado::EmJogo || vencedor_ < 0) return;
  int perdedor = 1 - vencedor_;
  vitorias_[vencedor_]--;
  vida_[perdedor] = vidaAntes_;
  veneno_[perdedor] = venenoAntes_;
  variacao_[perdedor] = 0;
  vencedor_ = -1;
  estado_ = Estado::EmJogo;
}

void MagicRegras::definirVidaInicial(int vida) {
  cfg_.vidaInicial = limitar(vida, 1, cfg_.vidaMax);
  novaSerie();
}

int MagicRegras::variacaoVisivel(int j, uint32_t agora) const {
  if (variacao_[j] == 0) return 0;
  if (agora - ultimaMudancaMs_[j] >= cfg_.janelaVariacaoMs) return 0;
  return variacao_[j];
}

}  // namespace placar
