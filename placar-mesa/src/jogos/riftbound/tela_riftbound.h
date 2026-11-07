#pragma once
// Riftbound 1x1: pontos até 8, série melhor de três, cronômetro da rodada.

#include "config_jogos.h"
#include "duelo_base.h"
#include "riftbound_regras.h"

namespace placar {

class TelaRiftbound : public DueloBase {
 public:
  explicit TelaRiftbound(Contexto& ctx, ConfigRiftbound cfg = {});

 protected:
  void desenharPainel(Pintor& p, Lado lado, uint32_t agoraMs) override;
  void aoEventoPainel(Lado lado, const Evento& e, Ponto local) override;
  void montarOpcoes(Lado lado, std::vector<ItemOpcao>& itens) override;

 private:
  void desenharEmJogo(Pintor& p, int j);
  void desenharResultado(Pintor& p, int j);
  void novaSerie();

  RiftboundRegras regras_;
};

}  // namespace placar
