#pragma once
// Registro de jogos: o menu é gerado a partir desta lista.
// Para adicionar um jogo, inclua uma linha em registro_jogos.cpp.

#include <memory>
#include <vector>

#include "contexto.h"
#include "tela.h"

namespace placar {

struct EntradaJogo {
  const char* nome;       // texto grande do botão no menu
  const char* subtitulo;  // texto pequeno embaixo do nome
  uint16_t cor;           // cor do botão
  const char* id;         // identificador curto (salvo na NVS como "último jogo")
  std::unique_ptr<Tela> (*criar)(Contexto&);
};

const std::vector<EntradaJogo>& jogosRegistrados();

// Fábrica genérica: criar<MinhaTela> constrói MinhaTela(Contexto&).
template <class T>
std::unique_ptr<Tela> criar(Contexto& ctx) {
  return std::unique_ptr<Tela>(new T(ctx));
}

}  // namespace placar
