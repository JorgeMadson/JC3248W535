#pragma once
// Tela AXS15231B via QSPI. Desenha-se num canvas em memória (PSRAM) e
// enviar() manda a tela inteira de uma vez (flush), que funciona bem nesta tela.

#include <Arduino_GFX_Library.h>

namespace placar {

class Display {
 public:
  bool iniciar();
  Arduino_GFX* canvas() { return canvas_; }
  void enviar() { canvas_->flush(); }

 private:
  Arduino_DataBus* bus_ = nullptr;
  Arduino_GFX* lcd_ = nullptr;
  Arduino_Canvas* canvas_ = nullptr;
};

}  // namespace placar
