#include "dialogos.h"

#include <memory>

#include "componentes.h"
#include "pintor.h"

namespace placar {

namespace {
constexpr int16_t MARGEM = 14;
constexpr int16_t TITULO_H = 58;
constexpr int16_t ITEM_H = 46;
constexpr int16_t ITEM_ESPACO = 6;
constexpr int16_t RAIO = 26;

// Folha escura de cantos redondos, com a "alça" no topo.
void desenharCaixa(Pintor& p, Retangulo c) {
  p.arredondado(c, RAIO, cor::DIALOGO);
  p.contornoArredondado(c, RAIO, cor::DIVISORIA);
  p.arredondado({int16_t(c.centroX() - 22), int16_t(c.y + 10), 44, 4}, 2, cor::DIVISORIA);
}

// Item da lista: fundo, texto à esquerda e uma seta.
void desenharItem(Pintor& p, Retangulo r, const char* texto, uint16_t corTexto) {
  p.arredondado(r, 14, cor::SUPERFICIE);
  p.texto(texto, int16_t(r.x + 16), r.centroY(), Fonte::Texto, corTexto, Alinhamento::Esquerda);
  int16_t sx = int16_t(r.direita() - 22), sy = r.centroY();
  p.triangulo({sx, int16_t(sy - 7)}, {sx, int16_t(sy + 7)}, {int16_t(sx + 8), sy}, cor::APAGADO);
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

// A folha sobe da borda de quem abriu (fica na metade dele, colada embaixo).
Retangulo TelaOpcoes::caixa() const {
  int16_t n = int16_t(itens_.size());
  int16_t h = TITULO_H + n * (ITEM_H + ITEM_ESPACO) + MARGEM;
  return {6, int16_t(ALTURA_TELA - 6 - h), int16_t(LARGURA_TELA - 12), h};
}

// Item i da lista; i == quantidade de itens é o "Fechar", no canto do título.
Retangulo TelaOpcoes::areaItem(size_t i) const {
  Retangulo c = caixa();
  if (i == itens_.size()) return {int16_t(c.direita() - 96), int16_t(c.y + 18), 90, 40};
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
  p.texto(titulo_.c_str(), int16_t(c.x + 22), int16_t(c.y + 38), Fonte::Titulo, cor::TEXTO, Alinhamento::Esquerda);
  Retangulo fechar = areaItem(itens_.size());
  p.texto("Fechar", int16_t(fechar.direita() - 10), fechar.centroY(), Fonte::Legenda, cor::SUAVE, Alinhamento::Direita);
  for (size_t i = 0; i < itens_.size(); i++) {
    // "Voltar ao menu" e afins perdem o placar: em vermelho
    bool perigo = itens_[i].texto.find("menu") != std::string::npos;
    desenharItem(p, areaItem(i), itens_[i].texto.c_str(), perigo ? cor::ALERTA : cor::TEXTO);
  }
  p.orientar(Lado::Baixo);
}

// ---------------- TelaConfirmar ----------------

namespace {
constexpr Retangulo CAIXA_CONFIRMAR{6, ALTURA_TELA - 6 - 200, LARGURA_TELA - 12, 200};
constexpr Retangulo BOTAO_NAO{MARGEM + 6, ALTURA_TELA - 6 - 68, 128, 50};
constexpr Retangulo BOTAO_SIM{LARGURA_TELA - MARGEM - 6 - 128, ALTURA_TELA - 6 - 68, 128, 50};
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
  desenharLinhas(p, pergunta_, CAIXA_CONFIRMAR.centroX(), CAIXA_CONFIRMAR.y + 66, Fonte::Texto, 30);
  Botao{BOTAO_NAO, "Não", cor::SUPERFICIE, Fonte::Texto}.desenhar(p);
  Botao{BOTAO_SIM, "Sim", cor::OURO, Fonte::Texto, cor::ESCURO_TEXTO}.desenhar(p);
  p.orientar(Lado::Baixo);
}

}  // namespace placar
