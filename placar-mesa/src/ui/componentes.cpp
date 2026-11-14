#include "componentes.h"

#include <stdio.h>

#include "layout_duelo.h"

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

  int16_t yTempo = a.y + 24;
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

  // Barra do tempo que resta (cheia = rodada inteira). Fica vermelha no fim.
  Retangulo barra{int16_t(a.x + 12), int16_t(a.y + 46), int16_t(a.w - 24), 4};
  Cronometro::Leitura l = c.ler(agora);
  p.retangulo(barra, cor::DIVISORIA);
  if (l.acrescimo) {
    p.retangulo(barra, cor::ALERTA);
  } else if (c.duracaoS() > 0) {
    int16_t w = int16_t(uint32_t(barra.w) * l.segundos / c.duracaoS());
    bool noFim = l.segundos * 10 <= c.duracaoS();  // últimos 10%
    if (w > 0) p.retangulo({barra.x, barra.y, w, barra.h}, noFim ? cor::ALERTA : (rodando ? cor::OURO : cor::APAGADO));
  }

  p.texto(rodando ? "toque para pausar" : "toque para iniciar", a.centroX(), a.y + 60, Fonte::Legenda, cor::APAGADO);
}

void desenharIconeOpcoes(Pintor& p, Retangulo a, uint16_t c) {
  p.contornoArredondado(a, 8, c);
  int16_t w = a.w / 2, x = a.centroX() - w / 2, y = a.centroY();
  for (int i = -1; i <= 1; i++) p.retangulo({x, int16_t(y + i * 8 - 1), w, 3}, c);
}

void desenharCartao(Pintor& p, uint16_t fundo, uint16_t contorno) {
  p.arredondado(layout::CARTAO, 14, fundo);
  p.contornoArredondado(layout::CARTAO, 14, contorno);
}

void desenharBotaoSinal(Pintor& p, Retangulo a, bool mais, uint16_t c, uint16_t corSinal) {
  p.arredondado(a, RAIO_BOTAO, c);
  int16_t lado = (a.w < a.h ? a.w : a.h) * 9 / 20;  // comprimento da barra
  int16_t grossura = lado / 4 < 4 ? 4 : lado / 4;
  int16_t cx = a.centroX(), cy = a.centroY();
  p.retangulo({int16_t(cx - lado / 2), int16_t(cy - grossura / 2), lado, grossura}, corSinal);
  if (mais) p.retangulo({int16_t(cx - grossura / 2), int16_t(cy - lado / 2), grossura, lado}, corSinal);
}

void desenharSegmentos(Pintor& p, Retangulo a, int cheios, int total, uint16_t corCheio, uint16_t corVazio) {
  if (total <= 0) return;
  constexpr int16_t ESPACO = 4;
  int16_t w = int16_t((a.w - ESPACO * (total - 1)) / total);
  for (int i = 0; i < total; i++) {
    Retangulo seg{int16_t(a.x + i * (w + ESPACO)), a.y, w, a.h};
    p.arredondado(seg, 3, i < cheios ? corCheio : corVazio);
  }
}

void desenharEtiqueta(Pintor& p, const char* texto, int16_t x, int16_t yCentro, uint16_t fundo, uint16_t corTexto) {
  int16_t w = int16_t(p.largura(texto, Fonte::Legenda) + 20);
  p.arredondado({x, int16_t(yCentro - 13), w, 26}, 13, fundo);
  p.texto(texto, int16_t(x + w / 2), yCentro, Fonte::Legenda, corTexto);
}

void desenharResultado(Pintor& p, bool venceu, bool serie, const char* detalhe) {
  desenharCartao(p, venceu ? cor::OURO_ESCURO : cor::DERROTA, venceu ? cor::OURO : cor::DIVISORIA);
  const char* titulo = venceu ? (serie ? "VENCEU A SÉRIE!" : "VENCEU A PARTIDA!") : (serie ? "Perdeu a série" : "Perdeu a partida");
  p.texto(titulo, LARGURA_TELA / 2, layout::PAINEL_Y + 34, Fonte::Titulo, venceu ? cor::OURO : cor::APAGADO);
  p.texto(detalhe, LARGURA_TELA / 2, layout::PAINEL_Y + 74, Fonte::Legenda, cor::TEXTO);
  Botao{layout::BOTAO_DESFAZER, "Desfazer", cor::BOTAO_NEUTRO, Fonte::Texto}.desenhar(p);
  Botao{layout::BOTAO_SEGUIR, serie ? "Nova série" : "Próxima partida", cor::OURO, Fonte::Texto, cor::FUNDO}.desenhar(p);
}

void desenharDestaque(Pintor& p, Retangulo a) {
  p.contornoArredondado(a.recuado(-2), RAIO_BOTAO + 2, cor::TEXTO);
  p.contornoArredondado(a.recuado(-3), RAIO_BOTAO + 3, cor::TEXTO);
}

void desenharBolinhas(Pintor& p, Ponto inicio, int vencidas, int total, uint16_t c) {
  for (int i = 0; i < total; i++) {
    Ponto centro{int16_t(inicio.x + i * 24), inicio.y};
    if (i < vencidas) p.circulo(centro, 8, c);
    else p.contornoCirculo(centro, 8, cor::APAGADO);
  }
}

}  // namespace placar
