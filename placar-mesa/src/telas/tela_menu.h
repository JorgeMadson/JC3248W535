#pragma once
// Tela inicial: um botão por jogo registrado, repetido para os dois lados.

#include "contexto.h"
#include "geometria.h"
#include "tela.h"

namespace placar {

class TelaMenu : public Tela {
 public:
  explicit TelaMenu(Contexto& ctx);
  void aoEvento(const Evento& e) override;
  void desenhar(Pintor& p) override;

 private:
  Retangulo areaBotao(size_t i) const;  // coordenadas locais (metade de baixo)

  Contexto& ctx_;
  int ultimo_ = -1;  // índice do último jogo escolhido (destacado)
};

}  // namespace placar
