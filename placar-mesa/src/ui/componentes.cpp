#include "componentes.h"

#include <stdio.h>

namespace placar {

static constexpr int16_t RAIO_BOTAO = 12;

void Botao::desenhar(Pintor& p) const {
  p.arredondado(area, RAIO_BOTAO, cor);
  p.texto(rotulo, area.centroX(), area.centroY(), fonte, corTexto);
}

void desenharPlacar(Pintor& p, int valor, Ponto centro, int16_t larguraMax, uint16_t c) {
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", valor);
  Fonte f = p.largura(buf, Fonte::Gigante) <= larguraMax ? Fonte::Gigante : Fonte::Grande;
  p.texto(buf, centro.x, centro.y, f, c);
}

void desenharCronometro(Pintor& p, const Cronometro& c, uint32_t agora, Retangulo a) {
  char buf[12];
  Cronometro::formatar(c.ler(agora), buf, sizeof(buf));
  bool rodando = c.rodando();
  uint16_t corTempo = c.esgotado(agora) ? cor::ALERTA : (rodando ? cor::TEXTO : cor::APAGADO);

  int16_t yTempo = a.y + 28;
  int16_t xTempo = a.centroX() + 10;
  p.texto(buf, xTempo, yTempo, Fonte::Tempo, corTempo);

  // ícone à esquerda do tempo
  int16_t ix = a.x + 12, iy = yTempo;
  if (rodando) {
    p.retangulo({ix, int16_t(iy - 9), 5, 18}, corTempo);
    p.retangulo({int16_t(ix + 9), int16_t(iy - 9), 5, 18}, corTempo);
  } else {
    p.triangulo({ix, int16_t(iy - 10)}, {ix, int16_t(iy + 10)}, {int16_t(ix + 15), iy}, corTempo);
  }
  p.texto(rodando ? "toque para pausar" : "toque para iniciar", a.centroX(), a.y + 59, Fonte::Legenda, cor::APAGADO);
}

void desenharIconeOpcoes(Pintor& p, Retangulo a, uint16_t c) {
  p.contornoArredondado(a, 8, c);
  int16_t w = a.w / 2, x = a.centroX() - w / 2, y = a.centroY();
  for (int i = -1; i <= 1; i++) p.retangulo({x, int16_t(y + i * 8 - 1), w, 3}, c);
}

void desenharBolinhas(Pintor& p, Ponto inicio, int vencidas, int total, uint16_t c) {
  for (int i = 0; i < total; i++) {
    Ponto centro{int16_t(inicio.x + i * 24), inicio.y};
    if (i < vencidas) p.circulo(centro, 8, c);
    else p.contornoCirculo(centro, 8, cor::APAGADO);
  }
}

}  // namespace placar
