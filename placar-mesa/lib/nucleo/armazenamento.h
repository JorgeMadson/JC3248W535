#pragma once
// Guarda configurações que sobrevivem ao desligar (na placa: NVS / Preferences).

#include <stdint.h>

namespace placar {

class Armazenamento {
 public:
  virtual ~Armazenamento() = default;
  virtual int32_t lerInt(const char* chave, int32_t padrao) = 0;
  virtual void gravarInt(const char* chave, int32_t valor) = 0;
};

// Implementação que não guarda nada (testes, ou se a NVS falhar).
class ArmazenamentoNulo : public Armazenamento {
 public:
  int32_t lerInt(const char*, int32_t padrao) override { return padrao; }
  void gravarInt(const char*, int32_t) override {}
};

}  // namespace placar
