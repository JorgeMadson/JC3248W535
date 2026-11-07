#pragma once
// Componentes visuais reutilizáveis, sem regra de jogo.

#include "cronometro.h"
#include "pintor.h"

namespace placar {

// Botão retangular arredondado com rótulo centralizado.
struct Botao {
  Retangulo area;
  const char* rotulo = "";
  uint16_t cor = cor::BOTAO_NEUTRO;
  Fonte fonte = Fonte::Texto;
  uint16_t corTexto = cor::TEXTO;

  void desenhar(Pintor& p) const;
  bool tocado(Ponto local) const { return area.contem(local); }
};

// Número grande centralizado; troca para uma fonte menor se não couber.
void desenharPlacar(Pintor& p, int valor, Ponto centro, int16_t larguraMax, uint16_t cor);

// Cronômetro com ícone de play/pause e dica, dentro de "area".
void desenharCronometro(Pintor& p, const Cronometro& c, uint32_t agoraMs, Retangulo area);

// Ícone "≡" (três barras) do botão de opções.
void desenharIconeOpcoes(Pintor& p, Retangulo area, uint16_t cor);

// Bolinhas de vitórias da série (cheias = vencidas).
void desenharBolinhas(Pintor& p, Ponto inicio, int vencidas, int total, uint16_t cor);

}  // namespace placar
