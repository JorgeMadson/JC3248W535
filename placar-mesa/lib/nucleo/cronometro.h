#pragma once
// Cronômetro regressivo da rodada. Depois de zerar, passa a contar o acréscimo.
// Não lê relógio sozinho: todo método recebe o instante atual (agoraMs).

#include <stddef.h>
#include <stdint.h>

namespace placar {

class Cronometro {
 public:
  struct Leitura {
    bool acrescimo = false;  // true = tempo da rodada acabou; "segundos" é o excedente
    uint32_t segundos = 0;   // restante (ou excedente, se acrescimo)
  };

  explicit Cronometro(uint32_t duracaoS) : duracaoS_(duracaoS) {}

  void iniciar(uint32_t agoraMs);
  void pausar(uint32_t agoraMs);
  void alternar(uint32_t agoraMs);  // inicia se pausado, pausa se rodando
  void zerar();                     // volta ao tempo cheio, pausado
  void definirDuracao(uint32_t duracaoS) { duracaoS_ = duracaoS; }

  bool rodando() const { return rodando_; }
  uint32_t duracaoS() const { return duracaoS_; }
  uint32_t decorridoMs(uint32_t agoraMs) const;
  bool esgotado(uint32_t agoraMs) const { return decorridoMs(agoraMs) / 1000 >= duracaoS_; }
  Leitura ler(uint32_t agoraMs) const;

  // "50:00" (restante) ou "+1:05" (acréscimo). buf com pelo menos 12 bytes.
  static void formatar(Leitura l, char* buf, size_t tamanho);

 private:
  uint32_t duracaoS_;
  bool rodando_ = false;
  uint32_t acumuladoMs_ = 0;  // tempo corrido até a última pausa
  uint32_t inicioMs_ = 0;     // instante do último "iniciar"
};

}  // namespace placar
