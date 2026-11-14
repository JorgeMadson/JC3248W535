// Freedoom na JC3248W535: liga a placa e roda o motor do Doom numa tarefa
// própria (o motor precisa de mais pilha do que o loop() do Arduino tem).
#include <Arduino.h>

#include "placa.h"

extern "C" {
void doomgeneric_Create(int argc, char **argv);
void doomgeneric_Tick(void);
}

// Tudo o que o motor imprime com printf vai para a serial e, durante a
// inicialização, também para a tela.
static int escreverSaida(void *, const char *texto, int tamanho) {
  placa_console(texto, tamanho);
  return tamanho;
}

static void tarefaDoom(void *) {
  stdout = funopen(nullptr, nullptr, escreverSaida, nullptr, nullptr);
  setvbuf(stdout, nullptr, _IOLBF, 256);
  stderr = stdout;

  const char *wad = placa_caminho_wad();
  if (!wad) {
    placa_tela_erro("SEM JOGO",
                    "Copie o freedoom1.wad para a\n"
                    "raiz do cartao microSD.\n\n"
                    "Ou grave o episodio 1 na\n"
                    "placa:\n"
                    " python3 ferramentas/montar_wad.py\n"
                    " pio run -t gravar_wad");
    placa_enviar();
    for (;;) delay(1000);
  }

  static char *args[] = {(char *)"doom", (char *)"-iwad", nullptr, nullptr};
  args[2] = (char *)wad;
  doomgeneric_Create(3, args);
  for (;;) doomgeneric_Tick();
}

void setup() {
  Serial.begin(115200);
  placa_iniciar();
  xTaskCreatePinnedToCore(tarefaDoom, "doom", 40 * 1024, nullptr, 1, nullptr, 1);
}

void loop() { delay(1000); }
