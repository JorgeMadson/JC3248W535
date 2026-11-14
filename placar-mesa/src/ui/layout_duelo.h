#pragma once
// Geometria do layout "duelo" (2 jogadores frente a frente + faixa do meio).
// Tudo em coordenadas LOCAIS do jogador: a metade dele é a de baixo.
//
//   y   0..217  painel do outro jogador (visto de cabeça pra baixo)
//   y 218..261  faixa do meio: pílula do tempo (a da esquerda é a "dele")
//   y 262..479  painel do jogador, um cartão de cantos redondos:
//                 linha de cima   (50 px)  nome, série (Magic: -5 e +5)
//                 área central   (124 px)  - [ número com brilho ] +
//                 linha de baixo  (44 px)  pontos / veneno, opções "•••"
// Tocar na metade esquerda da área central tira, na direita soma.

#include "geometria.h"

namespace placar {
namespace layout {

constexpr int16_t FAIXA_H = 44;
constexpr int16_t PAINEL_H = (ALTURA_TELA - FAIXA_H) / 2;  // 218
constexpr int16_t PAINEL_Y = ALTURA_TELA - PAINEL_H;       // 262

constexpr Retangulo FAIXA_MEIO{0, PAINEL_H, LARGURA_TELA, FAIXA_H};  // igual nos dois lados
// Pílula do tempo de quem está embaixo (a da direita é a do outro, girada).
constexpr Retangulo TEMPO{10, PAINEL_H + 6, 142, 32};

constexpr Retangulo PAINEL{0, PAINEL_Y, LARGURA_TELA, PAINEL_H};
constexpr Retangulo CARTAO{6, PAINEL_Y + 6, LARGURA_TELA - 12, PAINEL_H - 12};
constexpr int16_t RAIO_CARTAO = 26;
constexpr Retangulo LINHA_TOPO{0, PAINEL_Y, LARGURA_TELA, 50};
constexpr Retangulo AREA_CENTRAL{0, PAINEL_Y + 50, LARGURA_TELA, 124};
constexpr Retangulo LINHA_BASE{0, PAINEL_Y + 174, LARGURA_TELA, 44};

// Centro do número (e do brilho atrás dele)
constexpr Ponto CENTRO_NUMERO{LARGURA_TELA / 2, CARTAO.y + 104};
constexpr int16_t RAIO_BRILHO = 96;

// "-" e "+" nas bordas: o toque vale a metade da área central; isto é só o
// desenho e o retorno visual do toque.
constexpr Retangulo BOTAO_MENOS{14, CENTRO_NUMERO.y - 46, 56, 92};
constexpr Retangulo BOTAO_MAIS{LARGURA_TELA - 70, CENTRO_NUMERO.y - 46, 56, 92};
constexpr int16_t LARGURA_PLACAR = LARGURA_TELA - 2 * 76;  // espaço entre os sinais

// "•••" no canto de baixo à direita
constexpr Retangulo BOTAO_OPCOES{LARGURA_TELA - 66, PAINEL_Y + 176, 54, 36};

// Tela de resultado (fim de partida ou de série), igual em todos os jogos
constexpr Retangulo BOTAO_DESFAZER{CARTAO.x + 14, CARTAO.y + 128, 96, 44};
constexpr Retangulo BOTAO_SEGUIR{CARTAO.x + 118, CARTAO.y + 128, CARTAO.w - 132, 44};
// Vitórias da série, na linha de baixo
constexpr Ponto BOLINHAS{LARGURA_TELA / 2 - 9, CARTAO.y + CARTAO.h - 18};

}  // namespace layout
}  // namespace placar
