#pragma once
// Luz de fundo da tela (liga/desliga).

namespace placar {

class Backlight {
 public:
  void iniciar();
  void ligar(bool ligado);
};

}  // namespace placar
