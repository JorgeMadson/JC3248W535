// Placar de mesa - JC3248W535 (ESP32-S3 + tela 3.5" 320x480 AXS15231B)
//
// Só monta as peças e roda o laço: toque -> gestos -> telas -> desenho.
// Estrutura: lib/nucleo (lógica pura, testada no PC), src/hal (hardware),
// src/ui (desenho), src/telas (menu/diálogos), src/jogos (um por pasta).

#include <Arduino.h>

#include <memory>

#include "backlight.h"
#include "config_placa.h"
#include "contexto.h"
#include "display.h"
#include "gerenciador_telas.h"
#include "gestos.h"
#include "pintor.h"
#include "plataforma.h"
#include "tela_menu.h"
#include "toque.h"

using namespace placar;

static Display display;
static Toque toque;
static Backlight backlight;
static RelogioArduino relogio;
static ArmazenamentoNvs armazenamento;
static GerenciadorTelas telas;
static ReconhecedorGestos gestos;
static Contexto contexto{telas, relogio, armazenamento};
static Pintor* pintor = nullptr;

static constexpr uint32_t INTERVALO_LACO_MS = 10;

#if DEBUG_TOQUE
static Ponto ultimoToque;
static uint32_t toqueVisivelAte = 0;
static bool pontoDesenhado = false;
#endif

void setup() {
  Serial.begin(115200);
  // Espera um pouco o PC abrir a porta USB, para o log do boot não se perder.
  for (uint32_t t0 = millis(); !Serial && millis() - t0 < 1500;) delay(10);
  Serial.println("\nPlacar de mesa iniciando...");

  backlight.iniciar();
  if (!display.iniciar()) Serial.println("ERRO: falha ao iniciar a tela/canvas");
  else Serial.println("Tela OK");
  pintor = new Pintor(display.canvas());

  toque.iniciar();
  Serial.println("Toque OK");
  Serial.println(armazenamento.iniciar() ? "NVS OK" : "AVISO: NVS indisponível, configurações não serão salvas");

  telas.empilhar(std::unique_ptr<Tela>(new TelaMenu(contexto)));
  telas.aplicarPendentes();
  Serial.println("Menu pronto");
}

void loop() {
  uint32_t agora = relogio.agoraMs();

  Ponto p;
  bool tocando = toque.ler(p);
  Evento eventos[ReconhecedorGestos::MAX_EVENTOS];
  bool estavaPressionado = gestos.pressionado();
  int n = gestos.processar(tocando, p, agora, eventos);

#if DEBUG_TOQUE
  if (tocando && !estavaPressionado) {
    Serial.printf("toque x=%d y=%d\n", p.x, p.y);
    ultimoToque = p;
    toqueVisivelAte = agora + 600;
  }
#else
  (void)estavaPressionado;
#endif

  for (int i = 0; i < n; i++) {
    telas.aoEvento(eventos[i]);
    telas.aplicarPendentes();  // cada evento já vê a pilha atualizada
  }
  telas.atualizar(agora);

  bool desenhar = telas.precisaDesenhar();
#if DEBUG_TOQUE
  bool pontoVisivel = agora < toqueVisivelAte;
  if (pontoVisivel != pontoDesenhado) desenhar = true;
#endif

  if (desenhar) {
    telas.desenhar(*pintor);  // sempre a tela inteira (da última opaca ao topo)
#if DEBUG_TOQUE
    pontoDesenhado = pontoVisivel;
    if (pontoVisivel) {
      pintor->orientar(Lado::Baixo);
      pintor->circulo(ultimoToque, 8, cor::TEXTO);
      pintor->contornoCirculo(ultimoToque, 12, cor::OURO);
    }
#endif
    display.enviar();
  }
  delay(INTERVALO_LACO_MS);
}
