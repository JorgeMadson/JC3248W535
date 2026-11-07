#include "riftbound_regras.h"

namespace placar {

void RiftboundRegras::somar(int j, int delta) {
  if (estado_ != Estado::EmJogo || j < 0 || j > 1) return;
  int novo = pontos_[j] + delta;
  if (novo < 0) novo = 0;
  if (novo > cfg_.pontosVitoria) novo = cfg_.pontosVitoria;
  pontos_[j] = novo;

  if (novo >= cfg_.pontosVitoria) {
    vencedor_ = j;
    vitorias_[j]++;
    estado_ = vitorias_[j] >= cfg_.partidasParaSerie ? Estado::SerieEncerrada : Estado::PartidaEncerrada;
  }
}

void RiftboundRegras::proximaPartida() {
  if (estado_ != Estado::PartidaEncerrada) return;
  pontos_[0] = pontos_[1] = 0;
  vencedor_ = -1;
  estado_ = Estado::EmJogo;
}

void RiftboundRegras::zerarPontos() {
  if (estado_ != Estado::EmJogo) return;
  pontos_[0] = pontos_[1] = 0;
}

void RiftboundRegras::novaSerie() {
  pontos_[0] = pontos_[1] = 0;
  vitorias_[0] = vitorias_[1] = 0;
  vencedor_ = -1;
  estado_ = Estado::EmJogo;
}

void RiftboundRegras::desfazerVitoria() {
  if (estado_ == Estado::EmJogo || vencedor_ < 0) return;
  vitorias_[vencedor_]--;
  pontos_[vencedor_] = cfg_.pontosVitoria - 1;
  vencedor_ = -1;
  estado_ = Estado::EmJogo;
}

}  // namespace placar
