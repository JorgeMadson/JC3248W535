#pragma once
// Relógio injetável: a lógica nunca chama millis() direto, para poder ser
// testada no PC com um relógio falso.

#include <stdint.h>

namespace placar {

class Relogio {
 public:
  virtual ~Relogio() = default;
  virtual uint32_t agoraMs() const = 0;
};

// Relógio controlado à mão (testes e simulações).
class RelogioManual : public Relogio {
 public:
  uint32_t agoraMs() const override { return agora_; }
  void definir(uint32_t ms) { agora_ = ms; }
  void avancar(uint32_t ms) { agora_ += ms; }

 private:
  uint32_t agora_ = 0;
};

}  // namespace placar
