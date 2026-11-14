#pragma once
// Cores e tipografia usadas em todas as telas. Visual "Holofote" (14/11/2026):
// fundo quase preto, cada jogador com um brilho redondo na cor dele atrás do
// número, ouro só para série, "quase lá" e vitória.
// Prévia: placar-mesa/prototipo/visual-placar.html (proposta E).

#include <stdint.h>

namespace placar {

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return uint16_t(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

// Cor entre a e b (RGB565): t = 0 dá a, t = 256 dá b. Faz os tons "translúcidos"
// (brilho, contornos) sem precisar de transparência no Pintor.
constexpr uint16_t misturar(uint16_t a, uint16_t b, int t) {
  return uint16_t(((((a >> 11) & 0x1F) + ((((b >> 11) & 0x1F) - ((a >> 11) & 0x1F)) * t) / 256) << 11) |
                  ((((a >> 5) & 0x3F) + ((((b >> 5) & 0x3F) - ((a >> 5) & 0x3F)) * t) / 256) << 5) |
                  ((a & 0x1F) + (((b & 0x1F) - (a & 0x1F)) * t) / 256));
}

namespace cor {
constexpr uint16_t FUNDO = rgb(0, 0, 0);            // vão entre os cartões e faixa do meio
constexpr uint16_t CARTAO = rgb(14, 15, 21);        // fundo dos cartões
constexpr uint16_t SUPERFICIE = rgb(24, 26, 34);    // itens de lista, pílulas
constexpr uint16_t FAIXA_MEIO = rgb(0, 0, 0);
constexpr uint16_t TEXTO = rgb(240, 241, 246);
constexpr uint16_t APAGADO = rgb(110, 114, 130);
constexpr uint16_t SUAVE = rgb(140, 144, 160);      // legendas
constexpr uint16_t DIVISORIA = rgb(56, 60, 74);     // contornos neutros
constexpr uint16_t VAZIO = rgb(40, 43, 54);         // segmento/bolinha apagada
constexpr uint16_t OURO = rgb(242, 193, 78);
constexpr uint16_t OURO_ESCURO = rgb(60, 45, 10);
constexpr uint16_t ALERTA = rgb(255, 92, 92);
constexpr uint16_t POSITIVO = rgb(110, 220, 120);
constexpr uint16_t VENENO = rgb(110, 210, 90);
constexpr uint16_t DIALOGO = rgb(16, 17, 24);
constexpr uint16_t BOTAO_NEUTRO = rgb(32, 34, 44);
constexpr uint16_t DERROTA = rgb(14, 15, 21);
constexpr uint16_t ESCURO_TEXTO = rgb(20, 22, 30);  // texto sobre ouro

// Cores dos jogos no menu
constexpr uint16_t RIFTBOUND = rgb(40, 150, 220);
constexpr uint16_t MAGIC = rgb(230, 120, 40);
}  // namespace cor

// Paleta de cada jogador (índice 0 = metade de baixo, 1 = metade de cima).
struct CoresJogador {
  const char* nome;
  uint16_t forte;   // cor viva: brilho, contorno do cartão
  uint16_t escuro;  // contorno do cartão (vivo misturado ao fundo)
  uint16_t claro;   // número, nome, pontos feitos
};

constexpr CoresJogador CORES_JOGADOR[2] = {
    {"AZUL", rgb(41, 112, 245), misturar(cor::CARTAO, rgb(41, 112, 245), 115), rgb(132, 186, 255)},
    {"VERMELHO", rgb(232, 52, 64), misturar(cor::CARTAO, rgb(232, 52, 64), 115), rgb(255, 140, 140)},
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
