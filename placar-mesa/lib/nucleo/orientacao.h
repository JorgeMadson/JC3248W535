#pragma once
// Layout espelhado: cada jogador enxerga a tela "na orientação dele".
//
// O jogador de BAIXO lê a tela normalmente. O de CIMA está do outro lado da mesa,
// então tudo dele é desenhado girado 180°. Para os jogos não precisarem pensar
// nisso, todo desenho e todo teste de toque de um jogador é feito em
// coordenadas LOCAIS: (0,0) é o canto superior esquerdo do ponto de vista dele,
// e a "metade dele" é sempre a metade de baixo.

#include "geometria.h"

namespace placar {

enum class Lado : uint8_t { Baixo = 0, Cima = 1 };

constexpr int indice(Lado l) { return static_cast<int>(l); }
constexpr Lado ladoDoIndice(int i) { return i == 0 ? Lado::Baixo : Lado::Cima; }
constexpr Lado outroLado(Lado l) { return l == Lado::Baixo ? Lado::Cima : Lado::Baixo; }

// Rotação do canvas (Arduino_GFX::setRotation) para desenhar na orientação do lado.
constexpr uint8_t rotacaoDoLado(Lado l) { return l == Lado::Baixo ? 0 : 2; }

// Lado dono de um ponto da tela (metade de baixo / metade de cima).
inline Lado ladoDoPonto(Ponto tela) { return tela.y >= ALTURA_TELA / 2 ? Lado::Baixo : Lado::Cima; }

// Converte coordenada de TELA para a coordenada LOCAL do jogador (e vice-versa:
// a transformação é a própria inversa).
inline Ponto paraLocal(Lado l, Ponto tela) {
  if (l == Lado::Baixo) return tela;
  return {int16_t(LARGURA_TELA - 1 - tela.x), int16_t(ALTURA_TELA - 1 - tela.y)};
}

}  // namespace placar
