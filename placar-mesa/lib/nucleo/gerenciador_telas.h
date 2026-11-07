#pragma once
// Pilha de telas: menu -> jogo -> diálogo.
//
// Empilhar/desempilhar NÃO acontece na hora: as operações ficam pendentes e são
// aplicadas em aplicarPendentes(), chamado pelo loop depois de entregar os
// eventos. Assim uma tela pode pedir para se fechar dentro do próprio aoEvento
// sem ser destruída no meio do método.

#include <memory>
#include <vector>

#include "tela.h"

namespace placar {

class GerenciadorTelas {
 public:
  void empilhar(std::unique_ptr<Tela> t);
  void desempilhar();
  void trocarTopo(std::unique_ptr<Tela> t);  // desempilha e empilha outra
  void voltarAoInicio();                      // deixa só a primeira tela

  void aplicarPendentes();

  void aoEvento(const Evento& e);
  void atualizar(uint32_t agoraMs);
  bool precisaDesenhar() const;
  void desenhar(Pintor& p);  // da última tela opaca até o topo

  size_t tamanho() const { return pilha_.size(); }
  Tela* topo() const { return pilha_.empty() ? nullptr : pilha_.back().get(); }

 private:
  enum class Op : uint8_t { Empilhar, Desempilhar, VoltarAoInicio };
  struct Pendente {
    Op op;
    std::unique_ptr<Tela> tela;
  };

  size_t primeiraVisivel() const;

  std::vector<std::unique_ptr<Tela>> pilha_;
  std::vector<Pendente> pendentes_;
  bool tudoSujo_ = true;
};

}  // namespace placar
