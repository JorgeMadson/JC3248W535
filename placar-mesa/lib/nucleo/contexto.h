#pragma once
// O que toda tela recebe ao ser criada: navegação, relógio e armazenamento.

#include "armazenamento.h"
#include "gerenciador_telas.h"
#include "relogio.h"

namespace placar {

struct Contexto {
  GerenciadorTelas& telas;
  const Relogio& relogio;
  Armazenamento& armazenamento;
};

}  // namespace placar
