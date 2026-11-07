#include "tela_riftbound.h"

#include <stdio.h>

#include "componentes.h"
#include "layout_duelo.h"
#include "pintor.h"

namespace placar {

using Estado = RiftboundRegras::Estado;

namespace {
// Botões da tela de resultado (dentro do painel, coordenadas locais)
constexpr Retangulo BOTAO_DESFAZER{8, layout::PAINEL_Y + 112, 104, 48};
constexpr Retangulo BOTAO_SEGUIR{120, layout::PAINEL_Y + 112, LARGURA_TELA - 128, 48};
constexpr Ponto BOLINHAS{20, layout::LINHA_BASE.y + 22};
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

  // Linha de baixo: vitórias da série e nome
  const auto& cfg = regras_.config();
  desenharBolinhas(p, BOLINHAS, regras_.vitorias(j), cfg.partidasParaSerie, cor::OURO);
  p.texto(CORES_JOGADOR[j].nome, LARGURA_TELA / 2, layout::LINHA_BASE.centroY(), Fonte::Texto, cor::APAGADO);
}

void TelaRiftbound::desenharEmJogo(Pintor& p, int j) {
  const CoresJogador& c = CORES_JOGADOR[j];
  const auto& cfg = regras_.config();
  p.retangulo(layout::PAINEL, c.escuro);

  char buf[24];
  snprintf(buf, sizeof(buf), "Série %d-%d", regras_.vitorias(j), regras_.vitorias(1 - j));
  p.texto(buf, 16, layout::LINHA_TOPO.centroY(), Fonte::Legenda, cor::APAGADO, Alinhamento::Esquerda);
  snprintf(buf, sizeof(buf), "%d de %d pontos", regras_.pontos(j), cfg.pontosVitoria);
  p.texto(buf, LARGURA_TELA - 16, layout::LINHA_TOPO.centroY(), Fonte::Legenda, cor::APAGADO, Alinhamento::Direita);

  Botao{layout::BOTAO_MENOS, "-", c.forte, Fonte::Grande}.desenhar(p);
  Botao{layout::BOTAO_MAIS, "+", c.forte, Fonte::Grande}.desenhar(p);
  Ponto centro{int16_t(LARGURA_TELA / 2), layout::AREA_CENTRAL.centroY()};
  desenharPlacar(p, regras_.pontos(j), centro, layout::LARGURA_PLACAR, cor::TEXTO);
}

void TelaRiftbound::desenharResultado(Pintor& p, int j) {
  bool venceu = regras_.vencedor() == j;
  bool serie = regras_.estado() == Estado::SerieEncerrada;
  p.retangulo(layout::PAINEL, venceu ? cor::OURO_ESCURO : cor::DERROTA);

  const char* titulo = venceu ? (serie ? "VENCEU A SÉRIE!" : "VENCEU A PARTIDA!") : (serie ? "Perdeu a série" : "Perdeu a partida");
  p.texto(titulo, LARGURA_TELA / 2, layout::PAINEL_Y + 34, Fonte::Titulo, venceu ? cor::OURO : cor::APAGADO);

  char buf[32];
  snprintf(buf, sizeof(buf), "Pontos %d-%d  ·  Série %d-%d", regras_.pontos(j), regras_.pontos(1 - j), regras_.vitorias(j),
           regras_.vitorias(1 - j));
  p.texto(buf, LARGURA_TELA / 2, layout::PAINEL_Y + 80, Fonte::Legenda, cor::TEXTO);

  Botao{BOTAO_DESFAZER, "Desfazer", cor::BOTAO_NEUTRO, Fonte::Texto}.desenhar(p);
  Botao{BOTAO_SEGUIR, serie ? "Nova série" : "Próxima partida", cor::OURO, Fonte::Texto, cor::FUNDO}.desenhar(p);
}

// ---------- Toques ----------

void TelaRiftbound::aoEventoPainel(Lado lado, const Evento& e, Ponto local) {
  if (e.tipo != TipoEvento::Tap) return;
  int j = indice(lado);

  if (regras_.estado() == Estado::EmJogo) {
    // Metade esquerda do painel = -1, direita = +1 (alvo grande, fora a linha de baixo)
    if (local.y < layout::PAINEL_Y || local.y >= layout::LINHA_BASE.y) return;
    regras_.somar(j, local.x < LARGURA_TELA / 2 ? -1 : +1);
    marcarSuja();
    return;
  }

  if (BOTAO_DESFAZER.contem(local)) {
    regras_.desfazerVitoria();
    marcarSuja();
  } else if (BOTAO_SEGUIR.contem(local)) {
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
