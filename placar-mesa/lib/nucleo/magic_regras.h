#pragma once
// Regras do contador de vida de Magic: The Gathering (2 jogadores), em série
// melhor de três. Estado puro, sem desenho; o tempo entra como parâmetro (agoraMs).

#include "config_jogos.h"

namespace placar {

class MagicRegras {
 public:
  enum class Estado : uint8_t {
    EmJogo,            // partida em andamento
    PartidaEncerrada,  // alguém perdeu; aguardando "próxima partida"
    SerieEncerrada,    // alguém venceu a série; aguardando "nova série"
  };

  explicit MagicRegras(ConfigMagic cfg = {});

  // Muda a vida e acumula a "variação recente" (ex.: -7), que zera se ficar
  // janelaVariacaoMs sem mexer naquele jogador. Ignorado fora de EmJogo.
  // Vida em 0 (ou menos) encerra a partida: o outro jogador vence.
  void mudarVida(int jogador, int delta, uint32_t agoraMs);
  // Chegar a venenoParaPerder também encerra a partida. Ignorado fora de EmJogo.
  void mudarVeneno(int jogador, int delta);

  void zerar();                        // vida inicial e sem veneno; a série continua (só EmJogo)
  void proximaPartida();               // depois de PartidaEncerrada: zera vida, mantém a série
  void novaSerie();                    // zera tudo
  void desfazerDerrota();              // a derrota foi engano: volta ao valor de antes
  void definirVidaInicial(int vida);   // muda a vida inicial e começa nova série

  int vida(int j) const { return vida_[j]; }
  int veneno(int j) const { return veneno_[j]; }
  int vidaInicial() const { return cfg_.vidaInicial; }
  bool perdeu(int j) const { return vida_[j] <= 0 || veneno_[j] >= cfg_.venenoParaPerder; }

  int vitorias(int j) const { return vitorias_[j]; }
  int vencedor() const { return vencedor_; }  // -1 se ninguém
  Estado estado() const { return estado_; }

  // Variação acumulada ainda visível (0 = nada a mostrar).
  int variacaoVisivel(int j, uint32_t agoraMs) const;

  const ConfigMagic& config() const { return cfg_; }

 private:
  void reiniciarVida();
  void conferirDerrota(int j, int vidaAntes, int venenoAntes);

  ConfigMagic cfg_;
  int vida_[2];
  int veneno_[2];
  int variacao_[2];
  uint32_t ultimaMudancaMs_[2];

  int vitorias_[2] = {0, 0};
  int vencedor_ = -1;
  Estado estado_ = Estado::EmJogo;
  // Valores do perdedor logo antes da mudança fatal (para desfazer).
  int vidaAntes_ = 0;
  int venenoAntes_ = 0;
};

}  // namespace placar
