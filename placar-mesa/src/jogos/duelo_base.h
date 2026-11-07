#pragma once
// Base para jogos de 2 jogadores frente a frente (layout_duelo.h):
//   - roteia cada toque para o jogador dono daquela metade, já em coordenadas locais
//   - cronômetro na faixa do meio (toque inicia/pausa), uma cópia virada para cada lado
//   - botão de opções em cada painel, com "Zerar tempo" e "Voltar ao menu" prontos
//
// O jogo só implementa: desenharPainel, aoEventoPainel e montarOpcoes.

#include <functional>
#include <vector>

#include "contexto.h"
#include "cronometro.h"
#include "dialogos.h"
#include "orientacao.h"
#include "tela.h"

namespace placar {

class DueloBase : public Tela {
 public:
  DueloBase(Contexto& ctx, uint32_t tempoRodadaS);

  void aoEvento(const Evento& e) final;
  void atualizar(uint32_t agoraMs) final;
  void desenhar(Pintor& p) final;

 protected:
  // Desenha o painel do jogador (o Pintor já está na orientação dele).
  virtual void desenharPainel(Pintor& p, Lado lado, uint32_t agoraMs) = 0;
  // Toque no painel do jogador, fora do botão de opções. "local" = coordenada dele.
  virtual void aoEventoPainel(Lado lado, const Evento& e, Ponto local) = 0;
  // Itens específicos do jogo no menu de opções (antes dos itens comuns).
  virtual void montarOpcoes(Lado lado, std::vector<ItemOpcao>& itens) = 0;
  // Chamado a cada ciclo; devolve true se precisa redesenhar.
  virtual bool atualizarJogo(uint32_t /*agoraMs*/) { return false; }

  // Abre um diálogo de opções/confirmação virado para "lado".
  void abrirDialogo(Lado lado, std::string titulo, std::vector<ItemOpcao> itens);

  Contexto& ctx_;
  Cronometro cronometro_;

 private:
  void abrirOpcoes(Lado lado);

  uint32_t ultimoSegundo_ = UINT32_MAX;
};

}  // namespace placar
