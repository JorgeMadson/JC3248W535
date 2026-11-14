#include "duelo_base.h"

#include <memory>

#include "componentes.h"
#include "layout_duelo.h"
#include "pintor.h"

namespace placar {

namespace {
constexpr uint32_t DESTAQUE_MS = 140;
}

DueloBase::DueloBase(Contexto& ctx, uint32_t tempoRodadaS) : ctx_(ctx), cronometro_(tempoRodadaS) {}

void DueloBase::aoEvento(const Evento& e) {
  if (layout::FAIXA_MEIO.contem(e.ponto)) {
    if (e.tipo == TipoEvento::Tap) {
      cronometro_.alternar(e.ms);
      marcarSuja();
    }
    return;
  }
  Lado lado = ladoDoPonto(e.ponto);
  Ponto local = paraLocal(lado, e.ponto);
  if (layout::BOTAO_OPCOES.contem(local)) {
    if (e.tipo == TipoEvento::Tap) abrirOpcoes(lado);
    return;
  }
  aoEventoPainel(lado, e, local);
}

void DueloBase::atualizar(uint32_t agora) {
  uint32_t seg = cronometro_.decorridoMs(agora) / 1000;
  if (seg != ultimoSegundo_) {
    ultimoSegundo_ = seg;
    marcarSuja();
  }
  if (destaqueAtivo_ && agora - destaqueInicioMs_ >= DESTAQUE_MS) {
    destaqueAtivo_ = false;
    marcarSuja();
  }
  if (atualizarJogo(agora)) marcarSuja();
}

void DueloBase::desenhar(Pintor& p) {
  uint32_t agora = ctx_.relogio.agoraMs();
  p.orientar(Lado::Baixo);
  p.limpar(cor::FUNDO);

  for (int l = 0; l < 2; l++) {
    Lado lado = ladoDoIndice(l);
    p.orientar(lado);
    desenharPainel(p, lado, agora);
    if (destaqueAtivo_ && destaqueLado_ == lado) desenharDestaque(p, destaqueArea_);
    desenharIconeOpcoes(p, layout::BOTAO_OPCOES, cor::APAGADO);
    desenharCronometro(p, cronometro_, agora, layout::TEMPO);
  }
  p.orientar(Lado::Baixo);
}

void DueloBase::destacar(Lado lado, Retangulo area, uint32_t agora) {
  destaqueAtivo_ = true;
  destaqueLado_ = lado;
  destaqueArea_ = area;
  destaqueInicioMs_ = agora;
  marcarSuja();
}

void DueloBase::abrirDialogo(Lado lado, std::string titulo, std::vector<ItemOpcao> itens) {
  ctx_.telas.empilhar(std::unique_ptr<Tela>(new TelaOpcoes(ctx_, lado, std::move(titulo), std::move(itens))));
}

void DueloBase::abrirOpcoes(Lado lado) {
  std::vector<ItemOpcao> itens;
  montarOpcoes(lado, itens);
  itens.push_back({"Zerar tempo", "Zerar o cronômetro?", [this] {
                     cronometro_.zerar();
                     marcarSuja();
                   }});
  itens.push_back({"Voltar ao menu", "Voltar ao menu?\nO placar atual será perdido.", [this] { ctx_.telas.voltarAoInicio(); }});
  abrirDialogo(lado, "Opções", std::move(itens));
}

}  // namespace placar
