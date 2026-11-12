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
  // Quando o placar roda dentro de outro sistema (ex.: launcher LVGL), volta
  // para ele. nullptr = firmware próprio, o menu não mostra o botão "Sair".
  void (*sair)() = nullptr;
};

}  // namespace placar
