#pragma once
// Regras do Riftbound 1x1 em série melhor de três. Estado puro, sem desenho.

#include "config_jogos.h"

namespace placar {

class RiftboundRegras {
 public:
  enum class Estado : uint8_t {
    EmJogo,            // partida em andamento
    PartidaEncerrada,  // alguém chegou aos pontos; aguardando "próxima partida"
    SerieEncerrada,    // alguém venceu a série; aguardando "nova série"
  };

  explicit RiftboundRegras(ConfigRiftbound cfg = {}) : cfg_(cfg) {}

  // Soma (ou subtrai) pontos do jogador 0 ou 1. Ignorado fora de EmJogo.
  // Nunca fica abaixo de zero. Ao chegar a pontosVitoria encerra a partida.
  void somar(int jogador, int delta);

  void proximaPartida();          // zera pontos, mantém a série
  void zerarPontos();             // zera pontos da partida atual
  void novaSerie();               // zera tudo
  void desfazerVitoria();         // ponto de vitória foi engano: volta a EmJogo

  int pontos(int j) const { return pontos_[j]; }
  int vitorias(int j) const { return vitorias_[j]; }
  int vencedor() const { return vencedor_; }  // -1 se ninguém
  Estado estado() const { return estado_; }
  const ConfigRiftbound& config() const { return cfg_; }

 private:
  ConfigRiftbound cfg_;
  int pontos_[2] = {0, 0};
  int vitorias_[2] = {0, 0};
  int vencedor_ = -1;
  Estado estado_ = Estado::EmJogo;
};

}  // namespace placar
