#include "tela_riftbound.h"

#include <stdio.h>

#include "componentes.h"
#include "layout_duelo.h"
#include "pintor.h"

namespace placar {

using Estado = RiftboundRegras::Estado;

namespace {
// Linha de baixo, durante a partida: barrinhas com os pontos feitos (uma por ponto)
constexpr Retangulo BARRA_PONTOS{LARGURA_TELA / 2 - 98, layout::CARTAO.y + layout::CARTAO.h - 32, 196, 8};
// Linha de cima: nome à esquerda, vitórias da série à direita
constexpr Ponto NOME{layout::CARTAO.x + 20, layout::CARTAO.y + 24};
constexpr Ponto BOLINHAS_TOPO{layout::CARTAO.x + layout::CARTAO.w - 46, layout::CARTAO.y + 24};
}  // namespace

TelaRiftbound::TelaRiftbound(Contexto& ctx, ConfigRiftbound cfg) : DueloBase(ctx, cfg.tempoRodadaS), regras_(cfg) {}

void TelaRiftbound::novaSerie() {
  regras_.novaSerie();
  cronometro_.zerar();
  marcarSuja();
}

// ---------- Desenho ----------

void TelaRiftbound::desenharPainel(Pintor& p, Lado lado, uint32_t) {
  int j = indice(lado);
  if (regras_.estado() == Estado::EmJogo) desenharEmJogo(p, j);
  else desenharResultado(p, j);
}

void TelaRiftbound::desenharEmJogo(Pintor& p, int j) {
  const CoresJogador& c = CORES_JOGADOR[j];
  const auto& cfg = regras_.config();
  bool quaseLa = regras_.pontos(j) == cfg.pontosVitoria - 1;

  desenharCartao(p, c.escuro);
  desenharBrilho(p, layout::CENTRO_NUMERO, layout::RAIO_BRILHO, c.forte);

  p.texto(c.nome, NOME.x, NOME.y, Fonte::Legenda, c.claro, Alinhamento::Esquerda);
  desenharBolinhas(p, BOLINHAS_TOPO, regras_.vitorias(j), cfg.partidasParaSerie, cor::OURO);

  // Centro: - pontos +
  desenharSinal(p, {layout::BOTAO_MENOS.centroX(), layout::CENTRO_NUMERO.y}, false, cor::APAGADO);
  desenharSinal(p, {layout::BOTAO_MAIS.centroX(), layout::CENTRO_NUMERO.y}, true, cor::APAGADO);
  desenharPlacar(p, regras_.pontos(j), layout::CENTRO_NUMERO, layout::LARGURA_PLACAR, quaseLa ? cor::OURO : c.claro);

  // Embaixo: quanto falta para vencer
  desenharSegmentos(p, BARRA_PONTOS, regras_.pontos(j), cfg.pontosVitoria, quaseLa ? cor::OURO : c.claro, cor::VAZIO);
}

void TelaRiftbound::desenharResultado(Pintor& p, int j) {
  bool serie = regras_.estado() == Estado::SerieEncerrada;
  char buf[40];
  snprintf(buf, sizeof(buf), "%d a %d  ·  série %d a %d", regras_.pontos(j), regras_.pontos(1 - j), regras_.vitorias(j),
           regras_.vitorias(1 - j));
  placar::desenharResultado(p, regras_.vencedor() == j, serie, buf, regras_.vitorias(j),
                            regras_.config().partidasParaSerie);
}

// ---------- Toques ----------

void TelaRiftbound::aoEventoPainel(Lado lado, const Evento& e, Ponto local) {
  if (e.tipo != TipoEvento::Tap) return;
  int j = indice(lado);

  if (regras_.estado() == Estado::EmJogo) {
    // Metade esquerda do painel = -1, direita = +1 (alvo grande, fora a linha de baixo)
    if (local.y < layout::PAINEL_Y || local.y >= layout::LINHA_BASE.y) return;
    bool mais = local.x >= LARGURA_TELA / 2;
    regras_.somar(j, mais ? +1 : -1);
    destacar(lado, mais ? layout::BOTAO_MAIS : layout::BOTAO_MENOS, e.ms);
    return;
  }

  if (layout::BOTAO_DESFAZER.contem(local)) {
    regras_.desfazerVitoria();
    marcarSuja();
  } else if (layout::BOTAO_SEGUIR.contem(local)) {
    if (regras_.estado() == Estado::SerieEncerrada) novaSerie();
    else regras_.proximaPartida();
    marcarSuja();
  }
}

void TelaRiftbound::montarOpcoes(Lado, std::vector<ItemOpcao>& itens) {
  if (regras_.estado() == Estado::EmJogo) {
    itens.push_back({"Zerar pontos", "Zerar os pontos?\nA série continua como está.", [this] {
                       regras_.zerarPontos();
                       marcarSuja();
                     }});
  }
  itens.push_back({"Nova série", "Começar nova série?\nZera pontos, série e tempo.", [this] { novaSerie(); }});
}

}  // namespace placar
