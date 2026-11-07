#include "display.h"

#include "config_placa.h"
#include "geometria.h"

namespace placar {

bool Display::iniciar() {
  bus_ = new Arduino_ESP32QSPI(PINO_LCD_CS, PINO_LCD_SCK, PINO_LCD_D0, PINO_LCD_D1, PINO_LCD_D2, PINO_LCD_D3);
  lcd_ = new Arduino_AXS15231B(bus_, GFX_NOT_DEFINED, 0, false, LARGURA_TELA, ALTURA_TELA);
  canvas_ = new Arduino_Canvas(LARGURA_TELA, ALTURA_TELA, lcd_, 0, 0, 0);
  return canvas_->begin();
}

}  // namespace placar
