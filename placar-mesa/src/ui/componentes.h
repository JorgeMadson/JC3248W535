#pragma once
// Componentes visuais reutilizáveis, sem regra de jogo (visual "Holofote", ver tema.h).

#include "cronometro.h"
#include "pintor.h"

namespace placar {

// Botão em pílula com rótulo centralizado.
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

// Tempo numa pílula (layout::TEMPO): ícone de play/pause, mm:ss e uma barrinha do que resta.
void desenharCronometro(Pintor& p, const Cronometro& c, uint32_t agoraMs, Retangulo area);

// "•••" do botão de opções.
void desenharIconeOpcoes(Pintor& p, Retangulo area, uint16_t cor);

// Cartão do jogador (layout::CARTAO): fundo escuro e contorno fino.
void desenharCartao(Pintor& p, uint16_t contorno, uint8_t grossura = 1);

// Brilho redondo atrás do número: círculos do fundo do cartão até um tom escuro de "cor".
void desenharBrilho(Pintor& p, Ponto centro, int16_t raio, uint16_t cor);

// Sinal "-" ou "+" desenhado em barras finas (mais nítido que a fonte).
void desenharSinal(Pintor& p, Ponto centro, bool mais, uint16_t cor, int16_t lado = 20, int16_t grossura = 3);

// Barra de progresso em segmentos arredondados (ex.: pontos feitos de 8).
void desenharSegmentos(Pintor& p, Retangulo area, int cheios, int total, uint16_t corCheio, uint16_t corVazio);

// Painel de fim de partida/série: título, detalhe e os botões Desfazer / seguir
// (áreas em layout_duelo.h), mais as bolinhas da série.
void desenharResultado(Pintor& p, bool venceu, bool serie, const char* detalhe, int vitorias, int totalSerie);

// Retorno visual de um toque: contorno claro em volta da área por um instante.
void desenharDestaque(Pintor& p, Retangulo area);

// Bolinhas de vitórias da série (cheias = vencidas). "inicio" = centro da primeira.
void desenharBolinhas(Pintor& p, Ponto inicio, int vencidas, int total, uint16_t cor);

}  // namespace placar
