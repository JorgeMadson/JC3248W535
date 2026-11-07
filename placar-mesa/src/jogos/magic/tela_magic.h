#pragma once
// Magic: The Gathering: vida (20/30/40), variação recente, veneno, cronômetro.

#include "config_jogos.h"
#include "duelo_base.h"
#include "magic_regras.h"

namespace placar {

class TelaMagic : public DueloBase {
 public:
  explicit TelaMagic(Contexto& ctx, ConfigMagic cfg = {});

 protected:
  void desenharPainel(Pintor& p, Lado lado, uint32_t agoraMs) override;
  void aoEventoPainel(Lado lado, const Evento& e, Ponto local) override;
  void montarOpcoes(Lado lado, std::vector<ItemOpcao>& itens) override;
  bool atualizarJogo(uint32_t agoraMs) override;

 private:
  void abrirVidaInicial(Lado lado);

  MagicRegras regras_;
  bool variacaoDesenhada_[2] = {false, false};  // para saber quando o "-7" sumiu
};

}  // namespace placar
