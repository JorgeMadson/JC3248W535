#include "tela_menu.h"

#include <string.h>

#include "componentes.h"
#include "pintor.h"
#include "registro_jogos.h"

namespace placar {

namespace {
constexpr const char* CHAVE_ULTIMO = "ultimo_jogo";
constexpr int16_t METADE = ALTURA_TELA / 2;
constexpr int16_t TITULO_Y = METADE + 34;
constexpr int16_t BOTOES_Y = METADE + 54;
constexpr int16_t BOTOES_H = ALTURA_TELA - BOTOES_Y - 12;
constexpr int16_t ESPACO = 10;
constexpr int16_t MARGEM = 12;
// Botão "Sair" no meio da mesa (coordenadas de tela, só se ctx.sair).
constexpr Retangulo BOTAO_SAIR = {LARGURA_TELA / 2 - 40, METADE - 16, 80, 32};
}  // namespace

TelaMenu::TelaMenu(Contexto& ctx) : ctx_(ctx) {
  const auto& jogos = jogosRegistrados();
  int32_t salvo = ctx_.armazenamento.lerInt(CHAVE_ULTIMO, -1);
  if (salvo >= 0 && salvo < int32_t(jogos.size())) ultimo_ = int(salvo);
}

Retangulo TelaMenu::areaBotao(size_t i) const {
  size_t n = jogosRegistrados().size();
  int16_t h = int16_t((BOTOES_H - ESPACO * int16_t(n - 1)) / int16_t(n));
  return {MARGEM, int16_t(BOTOES_Y + i * (h + ESPACO)), int16_t(LARGURA_TELA - 2 * MARGEM), h};
}

void TelaMenu::aoEvento(const Evento& e) {
  if (e.tipo != TipoEvento::Tap) return;
  if (ctx_.sair && BOTAO_SAIR.recuado(-8).contem(e.ponto)) {
    ctx_.sair();
    return;
  }
  Lado lado = ladoDoPonto(e.ponto);
  Ponto local = paraLocal(lado, e.ponto);
  const auto& jogos = jogosRegistrados();
  for (size_t i = 0; i < jogos.size(); i++) {
    if (!areaBotao(i).contem(local)) continue;
    ultimo_ = int(i);
    ctx_.armazenamento.gravarInt(CHAVE_ULTIMO, int32_t(i));
    ctx_.telas.empilhar(jogos[i].criar(ctx_));
    marcarSuja();
    return;
  }
}

void TelaMenu::desenhar(Pintor& p) {
  p.orientar(Lado::Baixo);
  p.limpar(cor::FUNDO);

  const auto& jogos = jogosRegistrados();
  for (int l = 0; l < 2; l++) {
    p.orientar(ladoDoIndice(l));
    p.texto("ESCOLHA O JOGO", LARGURA_TELA / 2, TITULO_Y, Fonte::Legenda, cor::APAGADO);
    for (size_t i = 0; i < jogos.size(); i++) {
      Retangulo r = areaBotao(i);
      bool ultimo = int(i) == ultimo_;
      p.arredondado(r, 22, cor::CARTAO);
      // Monograma com brilho na cor do jogo
      Ponto m{int16_t(r.x + 46), r.centroY()};
      for (int k = 0; k < 6; k++) p.circulo(m, int16_t(34 - k * 5), misturar(cor::CARTAO, jogos[i].cor, 20 + k * 31));
      char letra[2] = {jogos[i].nome[0], 0};
      p.texto(letra, m.x, m.y, Fonte::Texto, cor::TEXTO);
      p.contornoArredondado(r, 22, ultimo ? cor::OURO : cor::DIVISORIA);
      if (ultimo) p.contornoArredondado(r.recuado(1), 21, cor::OURO);
      bool temEspaco = r.h >= 70;
      int16_t xTexto = r.x + 92;
      p.texto(jogos[i].nome, xTexto, r.centroY() - (temEspaco ? 12 : 0), Fonte::Titulo, cor::TEXTO, Alinhamento::Esquerda);
      if (temEspaco) p.texto(jogos[i].subtitulo, xTexto, r.centroY() + 20, Fonte::Legenda, cor::SUAVE, Alinhamento::Esquerda);
    }
  }
  p.orientar(Lado::Baixo);
  if (ctx_.sair) {
    p.arredondado(BOTAO_SAIR, BOTAO_SAIR.h / 2, cor::BOTAO_NEUTRO);
    p.contornoArredondado(BOTAO_SAIR, BOTAO_SAIR.h / 2, cor::DIVISORIA);
    p.texto("Sair", BOTAO_SAIR.centroX(), BOTAO_SAIR.centroY(), Fonte::Legenda, cor::TEXTO);
  }
}

}  // namespace placar
