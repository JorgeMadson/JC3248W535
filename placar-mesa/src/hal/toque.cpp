#include "toque.h"

#include <Wire.h>

#include "config_placa.h"

namespace placar {

void Toque::iniciar() {
  Wire.begin(PINO_TOQUE_SDA, PINO_TOQUE_SCL);
  Wire.setClock(400000);
}

bool Toque::ler(Ponto& p) {
  // Protocolo do AXS15231B: manda o comando de leitura e lê 8 bytes.
  static const uint8_t cmd[11] = {0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00};
  uint8_t buf[8] = {0};
  Wire.beginTransmission(ENDERECO_TOQUE);
  Wire.write(cmd, sizeof(cmd));
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom(ENDERECO_TOQUE, 8) != 8) return false;
  for (int i = 0; i < 8; i++) buf[i] = Wire.read();

  uint8_t n = buf[1];  // número de dedos
  if (n == 0 || n > 2) return false;
  int x = ((buf[2] & 0x0F) << 8) | buf[3];
  int y = ((buf[4] & 0x0F) << 8) | buf[5];
#if TOQUE_TROCA_XY
  int t = x; x = y; y = t;
#endif
#if TOQUE_INVERTE_X
  x = LARGURA_TELA - 1 - x;
#endif
#if TOQUE_INVERTE_Y
  y = ALTURA_TELA - 1 - y;
#endif
  if (x < 0 || x >= LARGURA_TELA || y < 0 || y >= ALTURA_TELA) return false;
  p = Ponto{int16_t(x), int16_t(y)};
  return true;
}

}  // namespace placar
