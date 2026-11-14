#pragma once
// Geometria do layout "duelo" (2 jogadores frente a frente + faixa do meio).
// Tudo em coordenadas LOCAIS do jogador: a metade dele é a de baixo.
//
//   y   0..204  painel do outro jogador (visto de cabeça pra baixo)
//   y 205..274  faixa do meio (cronômetro: metade esquerda é a cópia "dele")
//   y 275..479  painel do jogador:
//                 linha de cima   (44 px)  infos / botões pequenos
//                 área central   (116 px)  - [ placar ] +
//                 linha de baixo  (45 px)  infos / botão de opções

#include "geometria.h"

namespace placar {
namespace layout {

constexpr int16_t FAIXA_H = 70;
constexpr int16_t PAINEL_H = (ALTURA_TELA - FAIXA_H) / 2;  // 205
constexpr int16_t PAINEL_Y = ALTURA_TELA - PAINEL_H;       // 275

constexpr Retangulo FAIXA_MEIO{0, PAINEL_H, LARGURA_TELA, FAIXA_H};  // igual nos dois lados
constexpr Retangulo TEMPO{0, PAINEL_H, LARGURA_TELA / 2, FAIXA_H};   // cópia do jogador

constexpr Retangulo PAINEL{0, PAINEL_Y, LARGURA_TELA, PAINEL_H};
// O painel é desenhado como um cartão arredondado, com uma margem em volta.
constexpr Retangulo CARTAO{4, PAINEL_Y + 4, LARGURA_TELA - 8, PAINEL_H - 8};
constexpr Retangulo LINHA_TOPO{0, PAINEL_Y, LARGURA_TELA, 44};
constexpr Retangulo AREA_CENTRAL{0, PAINEL_Y + 44, LARGURA_TELA, 116};
constexpr Retangulo LINHA_BASE{0, PAINEL_Y + 160, LARGURA_TELA, 45};

// Botões +/- grandes nas laterais da área central
constexpr Retangulo BOTAO_MENOS{12, PAINEL_Y + 50, 72, 104};
constexpr Retangulo BOTAO_MAIS{LARGURA_TELA - 84, PAINEL_Y + 50, 72, 104};
constexpr int16_t LARGURA_PLACAR = LARGURA_TELA - 2 * 92;  // espaço entre os botões

constexpr Retangulo BOTAO_OPCOES{LARGURA_TELA - 66, PAINEL_Y + 158, 54, 38};

// Tela de resultado (fim de partida ou de série), igual em todos os jogos
constexpr Retangulo BOTAO_DESFAZER{12, PAINEL_Y + 104, 100, 48};
constexpr Retangulo BOTAO_SEGUIR{120, PAINEL_Y + 104, LARGURA_TELA - 132, 48};
// Vitórias da série e nome do jogador, na linha de baixo
constexpr Ponto BOLINHAS{24, PAINEL_Y + 177};

}  // namespace layout
}  // namespace placar
