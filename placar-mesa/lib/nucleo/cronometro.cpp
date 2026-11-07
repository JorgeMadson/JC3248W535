#include "cronometro.h"

#include <stdio.h>

namespace placar {

void Cronometro::iniciar(uint32_t agora) {
  if (rodando_) return;
  inicioMs_ = agora;
  rodando_ = true;
}

void Cronometro::pausar(uint32_t agora) {
  if (!rodando_) return;
  acumuladoMs_ += agora - inicioMs_;
  rodando_ = false;
}

void Cronometro::alternar(uint32_t agora) {
  if (rodando_) pausar(agora);
  else iniciar(agora);
}

void Cronometro::zerar() {
  rodando_ = false;
  acumuladoMs_ = 0;
}

uint32_t Cronometro::decorridoMs(uint32_t agora) const {
  return acumuladoMs_ + (rodando_ ? agora - inicioMs_ : 0);
}

Cronometro::Leitura Cronometro::ler(uint32_t agora) const {
  uint32_t s = decorridoMs(agora) / 1000;
  if (s < duracaoS_) return {false, duracaoS_ - s};
  return {true, s - duracaoS_};
}

void Cronometro::formatar(Leitura l, char* buf, size_t n) {
  unsigned long m = l.segundos / 60, s = l.segundos % 60;
  if (l.acrescimo) snprintf(buf, n, "+%lu:%02lu", m, s);
  else snprintf(buf, n, "%02lu:%02lu", m, s);
}

}  // namespace placar
