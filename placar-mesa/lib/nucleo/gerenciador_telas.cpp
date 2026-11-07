#include "gerenciador_telas.h"

namespace placar {

void GerenciadorTelas::empilhar(std::unique_ptr<Tela> t) { pendentes_.push_back({Op::Empilhar, std::move(t)}); }

void GerenciadorTelas::desempilhar() { pendentes_.push_back({Op::Desempilhar, nullptr}); }

void GerenciadorTelas::trocarTopo(std::unique_ptr<Tela> t) {
  desempilhar();
  empilhar(std::move(t));
}

void GerenciadorTelas::voltarAoInicio() { pendentes_.push_back({Op::VoltarAoInicio, nullptr}); }

void GerenciadorTelas::aplicarPendentes() {
  if (pendentes_.empty()) return;
  // As ações podem enfileirar novas operações (ex.: aoEntrar), então troca a fila.
  std::vector<Pendente> fila;
  fila.swap(pendentes_);
  Tela* topoAntes = topo();

  for (auto& p : fila) {
    switch (p.op) {
      case Op::Empilhar:
        if (p.tela) pilha_.push_back(std::move(p.tela));
        break;
      case Op::Desempilhar:
        if (pilha_.size() > 1) pilha_.pop_back();  // nunca remove a tela inicial
        break;
      case Op::VoltarAoInicio:
        while (pilha_.size() > 1) pilha_.pop_back();
        break;
    }
  }
  tudoSujo_ = true;
  if (topo() && topo() != topoAntes) topo()->aoEntrar();
}

void GerenciadorTelas::aoEvento(const Evento& e) {
  if (Tela* t = topo()) t->aoEvento(e);
}

void GerenciadorTelas::atualizar(uint32_t agora) {
  // Telas por baixo de um diálogo continuam vivas (ex.: cronômetro correndo).
  for (size_t i = primeiraVisivel(); i < pilha_.size(); i++) pilha_[i]->atualizar(agora);
}

bool GerenciadorTelas::precisaDesenhar() const {
  if (tudoSujo_) return true;
  for (size_t i = primeiraVisivel(); i < pilha_.size(); i++)
    if (pilha_[i]->suja()) return true;
  return false;
}

void GerenciadorTelas::desenhar(Pintor& p) {
  for (size_t i = primeiraVisivel(); i < pilha_.size(); i++) {
    pilha_[i]->desenhar(p);
    pilha_[i]->limparSuja();
  }
  tudoSujo_ = false;
}

size_t GerenciadorTelas::primeiraVisivel() const {
  if (pilha_.empty()) return 0;
  size_t i = pilha_.size() - 1;
  while (i > 0 && !pilha_[i]->opaca()) i--;
  return i;
}

}  // namespace placar
