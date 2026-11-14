#include "tela_magic.h"

#include <stdio.h>

#include "componentes.h"
#include "layout_duelo.h"
#include "pintor.h"

namespace placar {

using Estado = MagicRegras::Estado;

namespace {
constexpr const char* CHAVE_VIDA = "mtg_vida";

// Linha de cima: -5 / variação / +5
constexpr Retangulo BOTAO_MENOS5{12, layout::PAINEL_Y + 10, 72, 34};
constexpr Retangulo BOTAO_MAIS5{LARGURA_TELA - 84, layout::PAINEL_Y + 10, 72, 34};
// Entre os dois, quando não há variação para mostrar: vitórias da série
constexpr Ponto BOLINHAS_TOPO{LARGURA_TELA / 2 - 12, layout::PAINEL_Y + 27};
// Linha de baixo: veneno - / +
constexpr Retangulo VENENO_MENOS{12, layout::PAINEL_Y + 158, 44, 38};
constexpr Retangulo VENENO_MAIS{196, layout::PAINEL_Y + 158, 44, 38};
constexpr int16_t VENENO_TEXTO_X = (VENENO_MENOS.x + VENENO_MENOS.w + VENENO_MAIS.x) / 2;
constexpr int VIDA_BAIXA = 5;  // daqui para baixo a vida aparece em vermelho

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
  const auto& cfg = regras_.config();

  if (regras_.estado() != Estado::EmJogo) {
    variacaoDesenhada_[j] = false;
    bool serie = regras_.estado() == Estado::SerieEncerrada;
    char buf[48];
    snprintf(buf, sizeof(buf), "Vida %d-%d  ·  Série %d-%d", regras_.vida(j), regras_.vida(1 - j), regras_.vitorias(j),
             regras_.vitorias(1 - j));
    desenharResultado(p, regras_.vencedor() == j, serie, buf);
    desenharBolinhas(p, layout::BOLINHAS, regras_.vitorias(j), cfg.partidasParaSerie, cor::OURO);
    p.texto(c.nome, LARGURA_TELA / 2, layout::BOLINHAS.y, Fonte::Texto, cor::APAGADO);
    return;
  }

  desenharCartao(p, c.escuro, c.forte);

  // Linha de cima: botões de 5 e, no meio, a variação recente ou a série
  Botao{BOTAO_MENOS5, "-5", c.forte}.desenhar(p);
  Botao{BOTAO_MAIS5, "+5", c.forte}.desenhar(p);
  int var = regras_.variacaoVisivel(j, agora);
  variacaoDesenhada_[j] = var != 0;
  if (var != 0) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%+d", var);
    p.texto(buf, LARGURA_TELA / 2, BOLINHAS_TOPO.y, Fonte::Titulo, var < 0 ? cor::ALERTA : cor::POSITIVO);
  } else {
    desenharBolinhas(p, BOLINHAS_TOPO, regras_.vitorias(j), cfg.partidasParaSerie, cor::OURO);
  }

  // Centro: - vida +
  desenharBotaoSinal(p, layout::BOTAO_MENOS, false, c.forte);
  desenharBotaoSinal(p, layout::BOTAO_MAIS, true, c.forte);
  Ponto centro{int16_t(LARGURA_TELA / 2), layout::AREA_CENTRAL.centroY()};
  desenharPlacar(p, regras_.vida(j), centro, layout::LARGURA_PLACAR, regras_.vida(j) <= VIDA_BAIXA ? cor::ALERTA : cor::TEXTO);

  // Linha de baixo: veneno
  int veneno = regras_.veneno(j);
  uint16_t corVeneno = veneno >= cfg.venenoParaPerder - 2 ? cor::ALERTA : (veneno > 0 ? cor::VENENO : cor::APAGADO);
  desenharBotaoSinal(p, VENENO_MENOS, false, cor::BOTAO_NEUTRO);
  desenharBotaoSinal(p, VENENO_MAIS, true, cor::BOTAO_NEUTRO);
  p.texto(formatar("Veneno %d", veneno).c_str(), VENENO_TEXTO_X, VENENO_MENOS.centroY(), Fonte::Texto, corVeneno);
}

bool TelaMagic::atualizarJogo(uint32_t agora) {
  // Redesenha quando a variação recente some.
  for (int j = 0; j < 2; j++)
    if (variacaoDesenhada_[j] != (regras_.variacaoVisivel(j, agora) != 0)) return true;
  return false;
}

// ---------- Toques ----------

void TelaMagic::aoEventoPainel(Lado lado, const Evento& e, Ponto local) {
  int j = indice(lado);

  if (regras_.estado() != Estado::EmJogo) {
    if (e.tipo != TipoEvento::Tap) return;
    if (layout::BOTAO_DESFAZER.contem(local)) {
      regras_.desfazerDerrota();
      marcarSuja();
    } else if (layout::BOTAO_SEGUIR.contem(local)) {
      if (regras_.estado() == Estado::SerieEncerrada) novaSerie();
      else regras_.proximaPartida();
      marcarSuja();
    }
    return;
  }

  if (!e.ehPasso()) return;  // Tap, ou segurando (LongPress + Repeat): repete rápido

  if (BOTAO_MENOS5.contem(local)) {
    regras_.mudarVida(j, -5, e.ms);
    destacar(lado, BOTAO_MENOS5, e.ms);
  } else if (BOTAO_MAIS5.contem(local)) {
    regras_.mudarVida(j, +5, e.ms);
    destacar(lado, BOTAO_MAIS5, e.ms);
  } else if (VENENO_MENOS.contem(local)) {
    if (e.tipo != TipoEvento::Tap) return;
    regras_.mudarVeneno(j, -1);
    destacar(lado, VENENO_MENOS, e.ms);
  } else if (VENENO_MAIS.contem(local)) {
    if (e.tipo != TipoEvento::Tap) return;
    regras_.mudarVeneno(j, +1);
    destacar(lado, VENENO_MAIS, e.ms);
  } else if (local.y >= layout::AREA_CENTRAL.y && local.y < layout::LINHA_BASE.y) {
    // Metade esquerda da área central = -1, direita = +1 (alvo grande)
    bool mais = local.x >= LARGURA_TELA / 2;
    regras_.mudarVida(j, mais ? +1 : -1, e.ms);
    destacar(lado, mais ? layout::BOTAO_MAIS : layout::BOTAO_MENOS, e.ms);
  }
}

void TelaMagic::novaSerie() {
  regras_.novaSerie();
  cronometro_.zerar();
  marcarSuja();
}

void TelaMagic::montarOpcoes(Lado lado, std::vector<ItemOpcao>& itens) {
  int v = regras_.vidaInicial();
  if (regras_.estado() == Estado::EmJogo) {
    itens.push_back({"Zerar vida", formatar("Zerar a vida?\nOs dois voltam a %d. A série continua.", v), [this] {
                       regras_.zerar();
                       marcarSuja();
                     }});
  }
  itens.push_back({"Nova série", "Começar nova série?\nZera vida, série e tempo.", [this] { novaSerie(); }});
  itens.push_back({formatar("Vida inicial: %d", v), "", [this, lado] { abrirVidaInicial(lado); }});
}

void TelaMagic::abrirVidaInicial(Lado lado) {
  std::vector<ItemOpcao> itens;
  for (int v : regras_.config().opcoesVidaInicial) {
    std::string texto = formatar("%d de vida", v) + (v == 40 ? " (Commander)" : "");
    itens.push_back({texto, formatar("Recomeçar com %d de vida?\nComeça uma nova série.", v), [this, v] {
                       regras_.definirVidaInicial(v);
                       ctx_.armazenamento.gravarInt(CHAVE_VIDA, v);
                       marcarSuja();
                     }});
  }
  abrirDialogo(lado, "Vida inicial", std::move(itens));
}

}  // namespace placar
