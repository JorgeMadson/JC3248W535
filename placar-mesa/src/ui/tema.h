#pragma once
// Cores e tipografia usadas em todas as telas.

#include <stdint.h>

namespace placar {

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return uint16_t(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

namespace cor {
constexpr uint16_t FUNDO = rgb(10, 12, 20);
constexpr uint16_t FAIXA_MEIO = rgb(22, 25, 38);
constexpr uint16_t TEXTO = rgb(240, 240, 245);
constexpr uint16_t APAGADO = rgb(120, 125, 140);
constexpr uint16_t DIVISORIA = rgb(60, 64, 80);
constexpr uint16_t OURO = rgb(240, 190, 60);
constexpr uint16_t OURO_ESCURO = rgb(60, 45, 10);
constexpr uint16_t ALERTA = rgb(230, 70, 60);
constexpr uint16_t POSITIVO = rgb(80, 210, 110);
constexpr uint16_t VENENO = rgb(120, 200, 60);
constexpr uint16_t DIALOGO = rgb(32, 36, 52);
constexpr uint16_t BOTAO_NEUTRO = rgb(55, 60, 80);
constexpr uint16_t DERROTA = rgb(40, 40, 46);

// Cores dos jogos no menu
constexpr uint16_t RIFTBOUND = rgb(40, 110, 200);
constexpr uint16_t MAGIC = rgb(170, 90, 30);
}  // namespace cor

// Paleta de cada jogador (índice 0 = metade de baixo, 1 = metade de cima).
struct CoresJogador {
  const char* nome;
  uint16_t forte;   // botões, destaques
  uint16_t escuro;  // fundo do painel
};

constexpr CoresJogador CORES_JOGADOR[2] = {
    {"AZUL", rgb(30, 120, 220), rgb(12, 40, 80)},
    {"VERMELHO", rgb(220, 60, 60), rgb(80, 18, 22)},
};

// Tipografia: uma única família (Logisoso, lisa e condensada) em tamanhos fixos.
enum class Fonte : uint8_t {
  Gigante,  // placar principal (só números)
  Grande,   // placar com 3 dígitos, sinais de + e - (só números)
  Tempo,    // cronômetro (só números)
  Titulo,   // títulos e botões principais
  Texto,    // botões e textos comuns
  Legenda,  // dicas e textos pequenos
};

}  // namespace placar
