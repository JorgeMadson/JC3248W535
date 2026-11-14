// O ÚNICO lugar para registrar jogos. A ordem aqui é a ordem do menu.
#include "registro_jogos.h"

#include "magic/tela_magic.h"
#include "riftbound/tela_riftbound.h"
#include "tema.h"

namespace placar {

const std::vector<EntradaJogo>& jogosRegistrados() {
  static const std::vector<EntradaJogo> jogos = {
      {"Riftbound", "1x1 · melhor de 3", cor::RIFTBOUND, "rift", criar<TelaRiftbound>},
      {"Magic", "vida · melhor de 3", cor::MAGIC, "mtg", criar<TelaMagic>},
  };
  return jogos;
}

}  // namespace placar
