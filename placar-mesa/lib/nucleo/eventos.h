#pragma once
// Eventos de toque já interpretados (saída do ReconhecedorGestos).

#include "geometria.h"

namespace placar {

enum class TipoEvento : uint8_t {
  Tap,        // toque curto, disparado ao soltar
  LongPress,  // segurou além do tempo de toque longo (uma vez)
  Repeat,     // continua segurando depois do LongPress (periódico)
  Release,    // soltou (sempre, depois de qualquer um dos anteriores)
};

struct Evento {
  TipoEvento tipo = TipoEvento::Tap;
  Ponto ponto;          // onde o toque COMEÇOU, em coordenadas de tela
  uint32_t ms = 0;      // instante do evento

  // Toque curto ou repetição de quem está segurando: útil para botões +/-.
  bool ehPasso() const { return tipo == TipoEvento::Tap || tipo == TipoEvento::LongPress || tipo == TipoEvento::Repeat; }
};

}  // namespace placar
