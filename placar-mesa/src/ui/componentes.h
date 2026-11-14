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

// Fundo do painel de um jogador: cartão arredondado com contorno.
void desenharCartao(Pintor& p, uint16_t fundo, uint16_t contorno);

// Botão "+" ou "-" com o sinal desenhado em barras grossas (mais nítido que a fonte).
void desenharBotaoSinal(Pintor& p, Retangulo area, bool mais, uint16_t cor, uint16_t corSinal = cor::TEXTO);

// Barra de progresso em segmentos (ex.: pontos feitos de 8).
void desenharSegmentos(Pintor& p, Retangulo area, int cheios, int total, uint16_t corCheio, uint16_t corVazio);

// Nome do jogador numa etiqueta arredondada. "x" é a borda esquerda.
void desenharEtiqueta(Pintor& p, const char* texto, int16_t x, int16_t yCentro, uint16_t fundo, uint16_t corTexto);

// Painel de fim de partida/série: título, detalhe e os botões Desfazer / seguir
// (áreas em layout_duelo.h). Não desenha a linha de baixo.
void desenharResultado(Pintor& p, bool venceu, bool serie, const char* detalhe);

// Contorno claro em volta de um botão que acabou de ser tocado.
void desenharDestaque(Pintor& p, Retangulo area);

// Bolinhas de vitórias da série (cheias = vencidas).
void desenharBolinhas(Pintor& p, Ponto inicio, int vencidas, int total, uint16_t cor);

}  // namespace placar
