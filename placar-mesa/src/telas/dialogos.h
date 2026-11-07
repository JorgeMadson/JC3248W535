#pragma once
// Diálogos genéricos, sempre virados para o jogador que os abriu:
//   TelaOpcoes    lista de ações (cada uma pode pedir confirmação)
//   TelaConfirmar pergunta "Sim / Não"
// São telas não opacas: o jogo continua aparecendo (e o tempo correndo) por baixo.

#include <functional>
#include <string>
#include <vector>

#include "contexto.h"
#include "orientacao.h"
#include "tela.h"

namespace placar {

struct ItemOpcao {
  std::string texto;
  std::string pergunta;  // vazio = executa sem confirmar ("\n" quebra linha)
  std::function<void()> acao;
};

class TelaOpcoes : public Tela {
 public:
  TelaOpcoes(Contexto& ctx, Lado lado, std::string titulo, std::vector<ItemOpcao> itens);
  void aoEvento(const Evento& e) override;
  void desenhar(Pintor& p) override;
  bool opaca() const override { return false; }

 private:
  Retangulo caixa() const;
  Retangulo areaItem(size_t i) const;  // i == itens_.size() é o "Fechar"

  Contexto& ctx_;
  Lado lado_;
  std::string titulo_;
  std::vector<ItemOpcao> itens_;
};

class TelaConfirmar : public Tela {
 public:
  TelaConfirmar(Contexto& ctx, Lado lado, std::string pergunta, std::function<void()> aoConfirmar);
  void aoEvento(const Evento& e) override;
  void desenhar(Pintor& p) override;
  bool opaca() const override { return false; }

 private:
  Contexto& ctx_;
  Lado lado_;
  std::string pergunta_;
  std::function<void()> aoConfirmar_;
};

}  // namespace placar
