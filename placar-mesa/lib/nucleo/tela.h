#pragma once
// Interface de uma tela (menu, jogo, diálogo...). As telas ficam numa pilha
// controlada pelo GerenciadorTelas.

#include "eventos.h"

namespace placar {

class Pintor;  // definido na camada de UI (desenho real só existe na placa)

class Tela {
 public:
  virtual ~Tela() = default;

  virtual void aoEntrar() {}                        // ficou no topo da pilha
  virtual void aoEvento(const Evento& e) = 0;       // só a tela do topo recebe
  virtual void atualizar(uint32_t /*agoraMs*/) {}   // a cada ciclo (pode marcarSuja)
  virtual void desenhar(Pintor& p) = 0;

  // Telas não opacas (diálogos) são desenhadas por cima da tela de baixo.
  virtual bool opaca() const { return true; }

  void marcarSuja() { suja_ = true; }
  bool suja() const { return suja_; }
  void limparSuja() { suja_ = false; }

 private:
  bool suja_ = true;
};

}  // namespace placar
