#include "pintor.h"

#include <U8g2lib.h>

namespace placar {

static const uint8_t* fonteU8g2(Fonte f) {
  switch (f) {
    case Fonte::Gigante: return u8g2_font_logisoso92_tn;
    case Fonte::Grande: return u8g2_font_logisoso62_tn;
    case Fonte::Tempo: return u8g2_font_logisoso38_tn;
    case Fonte::Titulo: return u8g2_font_logisoso28_tf;
    case Fonte::Texto: return u8g2_font_logisoso20_tf;
    case Fonte::Legenda: return u8g2_font_logisoso16_tf;
  }
  return u8g2_font_logisoso20_tf;
}

// Medimos com a linha de base em Y_MEDIDA para as caixas não ficarem negativas.
static constexpr int16_t Y_MEDIDA = 200;

void Pintor::usarFonte(Fonte f) {
  g_->setFont(fonteU8g2(f));
  g_->setUTF8Print(true);
  g_->setTextWrap(false);
  g_->setTextSize(1);
}

int16_t Pintor::meiaAlturaReferencia(Fonte f) {
  int16_t& cache = cacheMeiaAltura_[static_cast<int>(f)];
  if (cache < 0) {
    int16_t x1, y1;
    uint16_t w, h;
    g_->getTextBounds("0", 0, Y_MEDIDA, &x1, &y1, &w, &h);
    cache = int16_t(Y_MEDIDA - y1) - int16_t(h / 2);
  }
  return cache;
}

int16_t Pintor::largura(const char* s, Fonte f) {
  usarFonte(f);
  int16_t x1, y1;
  uint16_t w, h;
  g_->getTextBounds(s, 0, Y_MEDIDA, &x1, &y1, &w, &h);
  return int16_t(w);
}

void Pintor::texto(const char* s, int16_t x, int16_t yCentro, Fonte f, uint16_t cor, Alinhamento a) {
  usarFonte(f);
  int16_t x1, y1;
  uint16_t w, h;
  g_->getTextBounds(s, 0, Y_MEDIDA, &x1, &y1, &w, &h);

  int16_t cx = x - x1;  // Esquerda
  if (a == Alinhamento::Centro) cx = x - x1 - int16_t(w / 2);
  if (a == Alinhamento::Direita) cx = x - x1 - int16_t(w);
  int16_t baseline = yCentro + meiaAlturaReferencia(f);

  g_->setTextColor(cor);
  g_->setCursor(cx, baseline);
  g_->print(s);
}

}  // namespace placar
