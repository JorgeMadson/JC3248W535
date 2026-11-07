#include "tela_magic.h"

#include <stdio.h>

#include "componentes.h"
#include "layout_duelo.h"
#include "pintor.h"

namespace placar {

namespace {
constexpr const char* CHAVE_VIDA = "mtg_vida";

// Linha de cima: -5 / variação / +5
constexpr Retangulo BOTAO_MENOS5{8, layout::PAINEL_Y + 4, 72, 36};
constexpr Retangulo BOTAO_MAIS5{LARGURA_TELA - 80, layout::PAINEL_Y + 4, 72, 36};
// Linha de baixo: veneno - / +
constexpr Retangulo VENENO_MENOS{8, layout::LINHA_BASE.y + 3, 44, 38};
constexpr Retangulo VENENO_MAIS{196, layout::LINHA_BASE.y + 3, 44, 38};
constexpr int16_t VENENO_TEXTO_X = (VENENO_MENOS.x + VENENO_MENOS.w + VENENO_MAIS.x) / 2;

std::string formatar(const char* fmt, int v) {
  char buf[64];
  snprintf(buf, sizeof(buf), fmt, v);
  return buf;
}
}  // namespace

TelaMagic::TelaMagic(Contexto& ctx, ConfigMagic cfg) : DueloBase(ctx, cfg.tempoRodadaS), regras_(cfg) {
  // Vida inicial salva (só aceita uma das opções conhecidas)
  int salva = int(ctx_.armazenamento.lerInt(CHAVE_VIDA, cfg.vidaInicial));
  for (int v : cfg.opcoesVidaInicial)
    if (v == salva) regras_.definirVidaInicial(v);
}

// ---------- Desenho ----------

void TelaMagic::desenharPainel(Pintor& p, Lado lado, uint32_t agora) {
  int j = indice(lado);
  const CoresJogador& c = CORES_JOGADOR[j];
  bool perdeu = regras_.perdeu(j);
  p.retangulo(layout::PAINEL, perdeu ? cor::DERROTA : c.escuro);

  // Linha de cima: botões de 5 e a variação recente (ou "DERROTA")
  Botao{BOTAO_MENOS5, "-5", c.forte}.desenhar(p);
  Botao{BOTAO_MAIS5, "+5", c.forte}.desenhar(p);
  int var = regras_.variacaoVisivel(j, agora);
  variacaoDesenhada_[j] = var != 0;
  if (var != 0) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%+d", var);
    p.texto(buf, LARGURA_TELA / 2, layout::LINHA_TOPO.centroY(), Fonte::Titulo, var < 0 ? cor::ALERTA : cor::POSITIVO);
  } else if (perdeu) {
    p.texto("DERROTA", LARGURA_TELA / 2, layout::LINHA_TOPO.centroY(), Fonte::Texto, cor::ALERTA);
  }

  // Centro: - vida +
  Botao{layout::BOTAO_MENOS, "-", c.forte, Fonte::Grande}.desenhar(p);
  Botao{layout::BOTAO_MAIS, "+", c.forte, Fonte::Grande}.desenhar(p);
  Ponto centro{int16_t(LARGURA_TELA / 2), layout::AREA_CENTRAL.centroY()};
  desenharPlacar(p, regras_.vida(j), centro, layout::LARGURA_PLACAR, perdeu ? cor::APAGADO : cor::TEXTO);

  // Linha de baixo: veneno
  int veneno = regras_.veneno(j);
  uint16_t corVeneno = veneno >= regras_.config().venenoParaPerder ? cor::ALERTA : (veneno > 0 ? cor::VENENO : cor::APAGADO);
  Botao{VENENO_MENOS, "-", cor::BOTAO_NEUTRO}.desenhar(p);
  Botao{VENENO_MAIS, "+", cor::BOTAO_NEUTRO}.desenhar(p);
  p.texto(formatar("Veneno %d", veneno).c_str(), VENENO_TEXTO_X, layout::LINHA_BASE.centroY(), Fonte::Texto, corVeneno);
}

bool TelaMagic::atualizarJogo(uint32_t agora) {
  // Redesenha quando a variação recente some.
  for (int j = 0; j < 2; j++)
    if (variacaoDesenhada_[j] != (regras_.variacaoVisivel(j, agora) != 0)) return true;
  return false;
}

// ---------- Toques ----------

void TelaMagic::aoEventoPainel(Lado lado, const Evento& e, Ponto local) {
  if (!e.ehPasso()) return;  // Tap, ou segurando (LongPress + Repeat): repete rápido
  int j = indice(lado);
  bool mudou = true;

  if (BOTAO_MENOS5.contem(local)) regras_.mudarVida(j, -5, e.ms);
  else if (BOTAO_MAIS5.contem(local)) regras_.mudarVida(j, +5, e.ms);
  else if (VENENO_MENOS.contem(local) && e.tipo == TipoEvento::Tap) regras_.mudarVeneno(j, -1);
  else if (VENENO_MAIS.contem(local) && e.tipo == TipoEvento::Tap) regras_.mudarVeneno(j, +1);
  else if (local.y >= layout::AREA_CENTRAL.y && local.y < layout::LINHA_BASE.y)
    regras_.mudarVida(j, local.x < LARGURA_TELA / 2 ? -1 : +1, e.ms);
  else mudou = false;

  if (mudou) marcarSuja();
}

void TelaMagic::montarOpcoes(Lado lado, std::vector<ItemOpcao>& itens) {
  int v = regras_.vidaInicial();
  itens.push_back({"Zerar vida", formatar("Zerar a vida?\nOs dois voltam a %d, sem veneno.", v), [this] {
                     regras_.zerar();
                     marcarSuja();
                   }});
  itens.push_back({formatar("Vida inicial: %d", v), "", [this, lado] { abrirVidaInicial(lado); }});
}

void TelaMagic::abrirVidaInicial(Lado lado) {
  std::vector<ItemOpcao> itens;
  for (int v : regras_.config().opcoesVidaInicial) {
    std::string texto = formatar("%d de vida", v) + (v == 40 ? " (Commander)" : "");
    itens.push_back({texto, formatar("Recomeçar com %d de vida?\nA vida atual será perdida.", v), [this, v] {
                       regras_.definirVidaInicial(v);
                       ctx_.armazenamento.gravarInt(CHAVE_VIDA, v);
                       marcarSuja();
                     }});
  }
  abrirDialogo(lado, "Vida inicial", std::move(itens));
}

}  // namespace placar
