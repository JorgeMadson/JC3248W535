#pragma once
// Regras do contador de vida de Magic: The Gathering (2 jogadores).
// Estado puro, sem desenho; o tempo entra como parâmetro (agoraMs).

#include "config_jogos.h"

namespace placar {

class MagicRegras {
 public:
  explicit MagicRegras(ConfigMagic cfg = {});

  // Muda a vida e acumula a "variação recente" (ex.: -7), que zera se ficar
  // janelaVariacaoMs sem mexer naquele jogador.
  void mudarVida(int jogador, int delta, uint32_t agoraMs);
  void mudarVeneno(int jogador, int delta);

  void zerar();                        // volta à vida inicial, sem veneno
  void definirVidaInicial(int vida);   // muda a vida inicial e zera

  int vida(int j) const { return vida_[j]; }
  int veneno(int j) const { return veneno_[j]; }
  int vidaInicial() const { return cfg_.vidaInicial; }
  bool perdeu(int j) const { return vida_[j] <= 0 || veneno_[j] >= cfg_.venenoParaPerder; }

  // Variação acumulada ainda visível (0 = nada a mostrar).
  int variacaoVisivel(int j, uint32_t agoraMs) const;

  const ConfigMagic& config() const { return cfg_; }

 private:
  ConfigMagic cfg_;
  int vida_[2];
  int veneno_[2];
  int variacao_[2];
  uint32_t ultimaMudancaMs_[2];
};

}  // namespace placar
