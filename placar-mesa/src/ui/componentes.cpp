#include "componentes.h"

#include <stdio.h>

#include "layout_duelo.h"

namespace placar {

void Botao::desenhar(Pintor& p) const {
  p.arredondado(area, area.h / 2, cor);
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
  Cronometro::Leitura l = c.ler(agora);
  Cronometro::formatar(l, buf, sizeof(buf));
  bool rodando = c.rodando();
  bool esgotado = c.esgotado(agora);
  uint16_t corTempo = esgotado ? cor::ALERTA : (rodando ? cor::TEXTO : cor::APAGADO);

  p.arredondado(a, a.h / 2, cor::BOTAO_NEUTRO);
  p.contornoArredondado(a, a.h / 2, cor::DIVISORIA);

  // ícone à esquerda
  int16_t ix = a.x + 16, iy = a.centroY() - 1;
  if (rodando) {
    p.retangulo({ix, int16_t(iy - 7), 4, 14}, corTempo);
    p.retangulo({int16_t(ix + 8), int16_t(iy - 7), 4, 14}, corTempo);
  } else {
    p.triangulo({ix, int16_t(iy - 8)}, {ix, int16_t(iy + 8)}, {int16_t(ix + 12), iy}, corTempo);
  }
  p.texto(buf, a.x + 82, iy, Fonte::Texto, corTempo);

  // Barrinha do tempo que resta (cheia = rodada inteira); vermelha no fim.
  Retangulo barra{int16_t(a.x + 40), int16_t(a.y + a.h - 6), 84, 2};
  p.retangulo(barra, cor::VAZIO);
  if (l.acrescimo) {
    p.retangulo(barra, cor::ALERTA);
  } else if (c.duracaoS() > 0) {
    int16_t w = int16_t(uint32_t(barra.w) * l.segundos / c.duracaoS());
    bool noFim = l.segundos * 10 <= c.duracaoS();  // últimos 10%
    if (w > 0) p.retangulo({barra.x, barra.y, w, barra.h}, noFim ? cor::ALERTA : (rodando ? cor::OURO : cor::APAGADO));
  }
}

void desenharIconeOpcoes(Pintor& p, Retangulo a, uint16_t c) {
  for (int i = -1; i <= 1; i++) p.circulo({int16_t(a.centroX() + i * 8), a.centroY()}, 2, c);
}

void desenharCartao(Pintor& p, uint16_t contorno, uint8_t grossura) {
  p.arredondado(layout::CARTAO, layout::RAIO_CARTAO, cor::CARTAO);
  for (uint8_t g = 0; g < grossura; g++)
    p.contornoArredondado(layout::CARTAO.recuado(g), layout::RAIO_CARTAO - g, contorno);
}

void desenharBrilho(Pintor& p, Ponto centro, int16_t raio, uint16_t c) {
  constexpr int ANEIS = 9;
  for (int k = 0; k < ANEIS; k++) {
    int16_t r = int16_t(raio - k * raio / ANEIS);
    p.circulo(centro, r, misturar(cor::CARTAO, c, 13 + k * 9));  // de ~5% a ~33% da cor
  }
}

void desenharSinal(Pintor& p, Ponto c, bool mais, uint16_t cor, int16_t lado, int16_t g) {
  p.retangulo({int16_t(c.x - lado / 2), int16_t(c.y - g / 2), lado, g}, cor);
  if (mais) p.retangulo({int16_t(c.x - g / 2), int16_t(c.y - lado / 2), g, lado}, cor);
}

void desenharSegmentos(Pintor& p, Retangulo a, int cheios, int total, uint16_t corCheio, uint16_t corVazio) {
  if (total <= 0) return;
  constexpr int16_t ESPACO = 4;
  int16_t w = int16_t((a.w - ESPACO * (total - 1)) / total);
  for (int i = 0; i < total; i++) {
    Retangulo seg{int16_t(a.x + i * (w + ESPACO)), a.y, w, a.h};
    p.arredondado(seg, a.h / 2, i < cheios ? corCheio : corVazio);
  }
}

void desenharResultado(Pintor& p, bool venceu, bool serie, const char* detalhe, int vitorias, int totalSerie) {
  const Retangulo& c = layout::CARTAO;
  desenharCartao(p, venceu ? cor::OURO : cor::DIVISORIA, venceu ? 2 : 1);
  if (venceu) desenharBrilho(p, {c.centroX(), int16_t(c.y + 62)}, 80, cor::OURO);

  const char* titulo = venceu ? (serie ? "VENCEU A SÉRIE" : "VITÓRIA") : (serie ? "PERDEU A SÉRIE" : "DERROTA");
  p.texto(titulo, c.centroX(), c.y + 58, Fonte::Titulo, venceu ? cor::OURO : cor::APAGADO);
  p.texto(detalhe, c.centroX(), c.y + 100, Fonte::Legenda, cor::SUAVE);
  Botao{layout::BOTAO_DESFAZER, "Desfazer", cor::BOTAO_NEUTRO, Fonte::Legenda}.desenhar(p);
  Botao{layout::BOTAO_SEGUIR, serie ? "Nova série" : "Próxima partida", cor::OURO, Fonte::Legenda, cor::ESCURO_TEXTO}
      .desenhar(p);
  desenharBolinhas(p, layout::BOLINHAS, vitorias, totalSerie, cor::OURO);
}

void desenharDestaque(Pintor& p, Retangulo a) {
  p.contornoArredondado(a, 16, cor::TEXTO);
  p.contornoArredondado(a.recuado(1), 15, cor::TEXTO);
}

void desenharBolinhas(Pintor& p, Ponto inicio, int vencidas, int total, uint16_t c) {
  for (int i = 0; i < total; i++) {
    Ponto centro{int16_t(inicio.x + i * 18), inicio.y};
    if (i < vencidas) {
      p.circulo(centro, 6, c);
    } else {
      p.contornoCirculo(centro, 6, cor::DIVISORIA);
      p.contornoCirculo(centro, 5, cor::DIVISORIA);
    }
  }
}

}  // namespace placar
