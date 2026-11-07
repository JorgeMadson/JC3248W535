#pragma once
// Tipos geométricos básicos e o tamanho da tela. Sem dependência de hardware.

#include <stdint.h>

namespace placar {

// Tela em retrato nativo (a placa fica deitada, cada jogador em uma ponta).
constexpr int16_t LARGURA_TELA = 320;
constexpr int16_t ALTURA_TELA = 480;

struct Ponto {
  int16_t x = 0;
  int16_t y = 0;
};

struct Retangulo {
  int16_t x = 0, y = 0, w = 0, h = 0;

  bool contem(Ponto p) const { return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h; }
  int16_t centroX() const { return x + w / 2; }
  int16_t centroY() const { return y + h / 2; }
  int16_t direita() const { return x + w; }
  int16_t base() const { return y + h; }
  // Retângulo encolhido "m" pixels de cada lado.
  Retangulo recuado(int16_t m) const { return {int16_t(x + m), int16_t(y + m), int16_t(w - 2 * m), int16_t(h - 2 * m)}; }
};

}  // namespace placar
