#include "tela_menu.h"

#include <string.h>

#include "componentes.h"
#include "pintor.h"
#include "registro_jogos.h"

namespace placar {

namespace {
constexpr const char* CHAVE_ULTIMO = "ultimo_jogo";
constexpr int16_t METADE = ALTURA_TELA / 2;
constexpr int16_t TITULO_Y = METADE + 28;
constexpr int16_t BOTOES_Y = METADE + 56;
constexpr int16_t BOTOES_H = ALTURA_TELA - BOTOES_Y - 12;
constexpr int16_t ESPACO = 10;
constexpr int16_t MARGEM = 16;
// Botão "Sair" sobre a divisória do meio (coordenadas de tela, só se ctx.sair).
constexpr Retangulo BOTAO_SAIR = {LARGURA_TELA / 2 - 44, METADE - 18, 88, 36};
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
  p.linhaH(24, METADE, LARGURA_TELA - 48, cor::DIVISORIA);

  const auto& jogos = jogosRegistrados();
  for (int l = 0; l < 2; l++) {
    p.orientar(ladoDoIndice(l));
    p.texto("Escolha o jogo", LARGURA_TELA / 2, TITULO_Y, Fonte::Texto, cor::APAGADO);
    for (size_t i = 0; i < jogos.size(); i++) {
      Retangulo r = areaBotao(i);
      p.arredondado(r, 16, jogos[i].cor);
      if (int(i) == ultimo_) p.contornoArredondado(r.recuado(-3), 18, cor::OURO);
      bool temEspaco = r.h >= 70;
      p.texto(jogos[i].nome, r.centroX(), r.centroY() - (temEspaco ? 12 : 0), Fonte::Titulo, cor::TEXTO);
      if (temEspaco) p.texto(jogos[i].subtitulo, r.centroX(), r.centroY() + 24, Fonte::Legenda, cor::TEXTO);
    }
  }
  p.orientar(Lado::Baixo);
  if (ctx_.sair) {
    p.arredondado(BOTAO_SAIR, 18, cor::BOTAO_NEUTRO);
    p.texto("Sair", BOTAO_SAIR.centroX(), BOTAO_SAIR.centroY(), Fonte::Legenda, cor::TEXTO);
  }
}

}  // namespace placar
