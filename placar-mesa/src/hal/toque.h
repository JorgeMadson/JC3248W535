#pragma once
// Toque capacitivo integrado no AXS15231B (I2C 0x3B).
// Já sai nas coordenadas da tela em retrato (0-319, 0-479).

#include "geometria.h"

namespace placar {

class Toque {
 public:
  void iniciar();
  // true se há um dedo na tela; "p" recebe a posição em coordenadas de tela.
  bool ler(Ponto& p);
};

}  // namespace placar
