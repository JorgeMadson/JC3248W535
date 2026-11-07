#pragma once
// Transforma leituras cruas do toque (tocando? x, y) em eventos, num só lugar.
//
//   Tap        soltou antes de tempoLongoMs
//   LongPress  completou tempoLongoMs segurando (uma vez)
//   Repeat     a cada intervaloRepeticaoMs depois do LongPress
//   Release    sempre ao soltar
//
// Debounce: o toque só é considerado solto depois de toleranciaSoltarMs sem
// contato, para leituras perdidas do controlador não virarem dois toques.

#include "eventos.h"

namespace placar {

struct ConfigGestos {
  uint32_t tempoLongoMs = 500;
  uint32_t intervaloRepeticaoMs = 120;
  uint32_t toleranciaSoltarMs = 40;
};

class ReconhecedorGestos {
 public:
  static constexpr int MAX_EVENTOS = 4;

  explicit ReconhecedorGestos(ConfigGestos cfg = {}) : cfg_(cfg) {}

  // Chamar a cada ciclo do loop. Escreve os eventos gerados em "saida" e
  // devolve quantos foram (no máximo MAX_EVENTOS).
  int processar(bool tocando, Ponto p, uint32_t agoraMs, Evento saida[MAX_EVENTOS]);

  bool pressionado() const { return pressionado_; }

 private:
  ConfigGestos cfg_;
  bool pressionado_ = false;
  bool longoDisparado_ = false;
  Ponto inicio_;
  uint32_t inicioMs_ = 0;
  uint32_t ultimoContatoMs_ = 0;
  uint32_t proximaRepeticaoMs_ = 0;
};

}  // namespace placar
