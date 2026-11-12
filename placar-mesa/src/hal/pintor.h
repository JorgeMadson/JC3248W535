#pragma once
// Fachada de desenho sobre o canvas. As telas só desenham através daqui, sempre
// em coordenadas locais da orientação atual (ver orientar()).

#include <Arduino_GFX_Library.h>

#include "geometria.h"
#include "orientacao.h"
#include "tema.h"

namespace placar {

enum class Alinhamento : uint8_t { Esquerda, Centro, Direita };

class Pintor {
 public:
  explicit Pintor(Arduino_GFX* g) : g_(g) {}

  // Passa a desenhar na orientação do jogador do lado "l".
  void orientar(Lado l) { g_->setRotation(rotacaoDoLado(l)); }

  void limpar(uint16_t c) { g_->fillScreen(c); }
  void retangulo(Retangulo r, uint16_t c) { g_->fillRect(r.x, r.y, r.w, r.h, c); }
  void arredondado(Retangulo r, int16_t raio, uint16_t c) { g_->fillRoundRect(r.x, r.y, r.w, r.h, raio, c); }
  void contornoArredondado(Retangulo r, int16_t raio, uint16_t c) { g_->drawRoundRect(r.x, r.y, r.w, r.h, raio, c); }
  void linhaH(int16_t x, int16_t y, int16_t w, uint16_t c) { g_->drawFastHLine(x, y, w, c); }
  void linhaV(int16_t x, int16_t y, int16_t h, uint16_t c) { g_->drawFastVLine(x, y, h, c); }
  void circulo(Ponto c, int16_t r, uint16_t cor) { g_->fillCircle(c.x, c.y, r, cor); }
  void contornoCirculo(Ponto c, int16_t r, uint16_t cor) { g_->drawCircle(c.x, c.y, r, cor); }
  void triangulo(Ponto a, Ponto b, Ponto c, uint16_t cor) { g_->fillTriangle(a.x, a.y, b.x, b.y, c.x, c.y, cor); }

  // Texto com o centro vertical em yCentro (altura de referência = altura dos
  // dígitos, para números e letras ficarem alinhados entre si). UTF-8.
  void texto(const char* s, int16_t x, int16_t yCentro, Fonte f, uint16_t cor, Alinhamento a = Alinhamento::Centro);
  int16_t largura(const char* s, Fonte f);

 private:
  void usarFonte(Fonte f);
  int16_t meiaAlturaReferencia(Fonte f);  // distância da linha de base ao centro dos dígitos

  Arduino_GFX* g_;
  int16_t cacheMeiaAltura_[6] = {-1, -1, -1, -1, -1, -1};
};

}  // namespace placar
