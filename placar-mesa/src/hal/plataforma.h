#pragma once
// Implementações de Relogio e Armazenamento para a placa (millis e NVS).

#include <Arduino.h>
#include <Preferences.h>

#include "armazenamento.h"
#include "relogio.h"

namespace placar {

class RelogioArduino : public Relogio {
 public:
  uint32_t agoraMs() const override { return millis(); }
};

// Guarda na NVS (partição "nvs"), no espaço de nomes "placar".
class ArmazenamentoNvs : public Armazenamento {
 public:
  bool iniciar() { return ok_ = prefs_.begin("placar", false); }

  int32_t lerInt(const char* chave, int32_t padrao) override { return ok_ ? prefs_.getInt(chave, padrao) : padrao; }

  void gravarInt(const char* chave, int32_t valor) override {
    if (ok_ && prefs_.getInt(chave, valor + 1) != valor) prefs_.putInt(chave, valor);  // evita gravar à toa
  }

 private:
  Preferences prefs_;
  bool ok_ = false;
};

}  // namespace placar
