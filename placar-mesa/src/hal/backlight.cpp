#include "backlight.h"

#include <Arduino.h>

#include "config_placa.h"

namespace placar {

void Backlight::iniciar() {
  pinMode(PINO_LCD_BL, OUTPUT);
  ligar(true);
}

void Backlight::ligar(bool ligado) { digitalWrite(PINO_LCD_BL, ligado ? HIGH : LOW); }

}  // namespace placar
