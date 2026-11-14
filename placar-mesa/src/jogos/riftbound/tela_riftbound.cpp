#include "tela_riftbound.h"

#include <stdio.h>

#include "componentes.h"
#include "layout_duelo.h"
#include "pintor.h"

namespace placar {

using Estado = RiftboundRegras::Estado;

namespace {
// Linha de baixo, durante a partida: barra com os pontos feitos (um segmento por ponto)
constexpr Retangulo BARRA_PONTOS{16, layout::PAINEL_Y + 170, 226, 14};
// Linha de cima: vitórias da série, à direita
constexpr Ponto BOLINHAS_TOPO{LARGURA_TELA - 52, layout::PAINEL_Y + 26};
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
  desenharCartao(p, c.escuro, c.forte);

  // Linha de cima: quem é o jogador e como está a série
  desenharEtiqueta(p, c.nome, 16, layout::PAINEL_Y + 26, c.forte, cor::TEXTO);
  p.texto("Série", BOLINHAS_TOPO.x - 18, BOLINHAS_TOPO.y, Fonte::Legenda, cor::APAGADO, Alinhamento::Direita);
  desenharBolinhas(p, BOLINHAS_TOPO, regras_.vitorias(j), cfg.partidasParaSerie, cor::OURO);

  // Centro: - pontos +
  desenharBotaoSinal(p, layout::BOTAO_MENOS, false, c.forte);
  desenharBotaoSinal(p, layout::BOTAO_MAIS, true, c.forte);
  Ponto centro{int16_t(LARGURA_TELA / 2), layout::AREA_CENTRAL.centroY()};
  bool quaseLa = regras_.pontos(j) == cfg.pontosVitoria - 1;
  desenharPlacar(p, regras_.pontos(j), centro, layout::LARGURA_PLACAR, quaseLa ? cor::OURO : cor::TEXTO);

  // Linha de baixo: quanto falta para vencer
  desenharSegmentos(p, BARRA_PONTOS, regras_.pontos(j), cfg.pontosVitoria, quaseLa ? cor::OURO : c.claro, cor::FUNDO);
}

void TelaRiftbound::desenharResultado(Pintor& p, int j) {
  bool serie = regras_.estado() == Estado::SerieEncerrada;
  char buf[40];
  snprintf(buf, sizeof(buf), "Pontos %d-%d  ·  Série %d-%d", regras_.pontos(j), regras_.pontos(1 - j), regras_.vitorias(j),
           regras_.vitorias(1 - j));
  placar::desenharResultado(p, regras_.vencedor() == j, serie, buf);

  desenharBolinhas(p, layout::BOLINHAS, regras_.vitorias(j), regras_.config().partidasParaSerie, cor::OURO);
  p.texto(CORES_JOGADOR[j].nome, LARGURA_TELA / 2, layout::BOLINHAS.y, Fonte::Texto, cor::APAGADO);
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
