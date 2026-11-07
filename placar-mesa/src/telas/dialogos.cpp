#include "dialogos.h"

#include <memory>

#include "componentes.h"
#include "pintor.h"

namespace placar {

namespace {
constexpr int16_t MARGEM = 14;
constexpr int16_t TITULO_H = 50;
constexpr int16_t ITEM_H = 50;
constexpr int16_t ITEM_ESPACO = 8;

void desenharCaixa(Pintor& p, Retangulo c) {
  p.arredondado(c, 16, cor::DIALOGO);
  p.contornoArredondado(c, 16, cor::OURO);
  p.contornoArredondado(c.recuado(1), 15, cor::OURO);
}

// Desenha linhas separadas por '\n', centradas em torno de yCentro.
void desenharLinhas(Pintor& p, const std::string& s, int16_t x, int16_t yCentro, Fonte f, int16_t alturaLinha) {
  std::vector<std::string> linhas;
  size_t ini = 0;
  for (;;) {
    size_t fim = s.find('\n', ini);
    linhas.push_back(s.substr(ini, fim == std::string::npos ? std::string::npos : fim - ini));
    if (fim == std::string::npos) break;
    ini = fim + 1;
  }
  int16_t y = yCentro - int16_t((linhas.size() - 1) * alturaLinha / 2);
  for (size_t i = 0; i < linhas.size(); i++) {
    p.texto(linhas[i].c_str(), x, y, i == 0 ? f : Fonte::Legenda, i == 0 ? cor::TEXTO : cor::APAGADO);
    y += alturaLinha;
  }
}
}  // namespace

// ---------------- TelaOpcoes ----------------

TelaOpcoes::TelaOpcoes(Contexto& ctx, Lado lado, std::string titulo, std::vector<ItemOpcao> itens)
    : ctx_(ctx), lado_(lado), titulo_(std::move(titulo)), itens_(std::move(itens)) {}

Retangulo TelaOpcoes::caixa() const {
  int16_t n = int16_t(itens_.size() + 1);  // + "Fechar"
  int16_t h = TITULO_H + n * (ITEM_H + ITEM_ESPACO) + MARGEM;
  return {MARGEM, int16_t((ALTURA_TELA - h) / 2), int16_t(LARGURA_TELA - 2 * MARGEM), h};
}

Retangulo TelaOpcoes::areaItem(size_t i) const {
  Retangulo c = caixa();
  return {int16_t(c.x + MARGEM), int16_t(c.y + TITULO_H + i * (ITEM_H + ITEM_ESPACO)), int16_t(c.w - 2 * MARGEM), ITEM_H};
}

void TelaOpcoes::aoEvento(const Evento& e) {
  if (e.tipo != TipoEvento::Tap) return;
  Ponto local = paraLocal(lado_, e.ponto);

  if (!caixa().contem(local) || areaItem(itens_.size()).contem(local)) {  // fora ou "Fechar"
    ctx_.telas.desempilhar();
    return;
  }
  for (size_t i = 0; i < itens_.size(); i++) {
    if (!areaItem(i).contem(local)) continue;
    ItemOpcao& item = itens_[i];
    if (!item.pergunta.empty()) {
      ctx_.telas.trocarTopo(std::unique_ptr<Tela>(new TelaConfirmar(ctx_, lado_, item.pergunta, item.acao)));
    } else {
      ctx_.telas.desempilhar();
      if (item.acao) item.acao();
    }
    return;
  }
}

void TelaOpcoes::desenhar(Pintor& p) {
  p.orientar(lado_);
  Retangulo c = caixa();
  desenharCaixa(p, c);
  p.texto(titulo_.c_str(), c.centroX(), c.y + TITULO_H / 2 + 2, Fonte::Titulo, cor::OURO);
  for (size_t i = 0; i < itens_.size(); i++) Botao{areaItem(i), itens_[i].texto.c_str()}.desenhar(p);
  Botao{areaItem(itens_.size()), "Fechar", cor::DIALOGO, Fonte::Texto, cor::APAGADO}.desenhar(p);
  p.contornoArredondado(areaItem(itens_.size()), 12, cor::DIVISORIA);
  p.orientar(Lado::Baixo);
}

// ---------------- TelaConfirmar ----------------

namespace {
constexpr Retangulo CAIXA_CONFIRMAR{MARGEM, 140, LARGURA_TELA - 2 * MARGEM, 200};
constexpr Retangulo BOTAO_NAO{MARGEM * 2, 270, 120, 54};
constexpr Retangulo BOTAO_SIM{LARGURA_TELA - MARGEM * 2 - 120, 270, 120, 54};
}  // namespace

TelaConfirmar::TelaConfirmar(Contexto& ctx, Lado lado, std::string pergunta, std::function<void()> aoConfirmar)
    : ctx_(ctx), lado_(lado), pergunta_(std::move(pergunta)), aoConfirmar_(std::move(aoConfirmar)) {}

void TelaConfirmar::aoEvento(const Evento& e) {
  if (e.tipo != TipoEvento::Tap) return;
  Ponto local = paraLocal(lado_, e.ponto);
  if (BOTAO_SIM.contem(local)) {
    ctx_.telas.desempilhar();
    if (aoConfirmar_) aoConfirmar_();
  } else if (BOTAO_NAO.contem(local) || !CAIXA_CONFIRMAR.contem(local)) {
    ctx_.telas.desempilhar();
  }
}

void TelaConfirmar::desenhar(Pintor& p) {
  p.orientar(lado_);
  desenharCaixa(p, CAIXA_CONFIRMAR);
  desenharLinhas(p, pergunta_, CAIXA_CONFIRMAR.centroX(), CAIXA_CONFIRMAR.y + 62, Fonte::Texto, 30);
  Botao{BOTAO_NAO, "Não", cor::BOTAO_NEUTRO, Fonte::Titulo}.desenhar(p);
  Botao{BOTAO_SIM, "Sim", cor::OURO, Fonte::Titulo, cor::FUNDO}.desenhar(p);
  p.orientar(Lado::Baixo);
}

}  // namespace placar
