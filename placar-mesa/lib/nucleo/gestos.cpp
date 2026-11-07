#include "gestos.h"

namespace placar {

int ReconhecedorGestos::processar(bool tocando, Ponto p, uint32_t agora, Evento saida[MAX_EVENTOS]) {
  int n = 0;
  auto emitir = [&](TipoEvento t) {
    if (n < MAX_EVENTOS) saida[n++] = Evento{t, inicio_, agora};
  };

  if (tocando) {
    if (!pressionado_) {  // começou um toque novo
      pressionado_ = true;
      longoDisparado_ = false;
      inicio_ = p;
      inicioMs_ = agora;
    }
    ultimoContatoMs_ = agora;

    if (!longoDisparado_ && agora - inicioMs_ >= cfg_.tempoLongoMs) {
      longoDisparado_ = true;
      proximaRepeticaoMs_ = agora + cfg_.intervaloRepeticaoMs;
      emitir(TipoEvento::LongPress);
    } else if (longoDisparado_ && agora >= proximaRepeticaoMs_) {
      // No máximo uma repetição por ciclo; reagenda a partir de agora para
      // não "atropelar" se o loop atrasar.
      proximaRepeticaoMs_ = agora + cfg_.intervaloRepeticaoMs;
      emitir(TipoEvento::Repeat);
    }
    return n;
  }

  // Sem contato: só solta depois da tolerância (debounce).
  if (pressionado_ && agora - ultimoContatoMs_ >= cfg_.toleranciaSoltarMs) {
    if (!longoDisparado_) emitir(TipoEvento::Tap);
    emitir(TipoEvento::Release);
    pressionado_ = false;
  }
  return n;
}

}  // namespace placar
