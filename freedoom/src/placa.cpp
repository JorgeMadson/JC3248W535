// Hardware da JC3248W535: tela, toque, microSD e partição de saves.
#include "placa.h"

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <FS.h>
#include <LittleFS.h>
#include <SD.h>
#include <SD_MMC.h>
#include <SPI.h>
#include <Wire.h>
#include <driver/i2s.h>
#include <esp_partition.h>

#include "config_placa.h"

static Arduino_DataBus *barramento = nullptr;
static Arduino_GFX *lcd = nullptr;
static Arduino_Canvas *tela = nullptr;  // imagem em memória + flush()
static SPIClass spiCartao(HSPI);        // a tela ocupa o outro barramento

static bool consoleAtivo = true;
static uint32_t consoleUltimoEnvio = 0;
static bool temCartao = false;
static fs::FS *cartao = nullptr;  // SD_MMC ou SD, o que tiver funcionado
static bool temSaves = false;
static char caminhoWad[64] = "";
static char pastaDados[32] = "";

static const uint16_t COR_BOTAO = 0x528A;  // cinza escuro

// ---------------------------------------------------------------- tela

uint16_t *placa_fb(void) { return tela->getFramebuffer(); }

void placa_enviar(void) { tela->flush(); }

static void rotulo(int x, int y, const char *texto) {
  tela->setTextSize(1);
  tela->setTextColor(COR_BOTAO);
  tela->setCursor(x, y);
  tela->print(texto);
}

void placa_desenhar_botoes(int modo) {
#if MOSTRAR_BOTOES
  // Mesmas áreas de comum/controles.c
  tela->drawRoundRect(2, 2, 76, 60, 8, COR_BOTAO);
  rotulo(26, 28, "MENU");
  if (modo == 1) {
    tela->drawRoundRect(402, 2, 76, 60, 8, COR_BOTAO);
    rotulo(428, 28, "MAPA");
    tela->drawRoundRect(244, 68, 114, 78, 8, COR_BOTAO);
    rotulo(289, 103, "ARMA");
    tela->drawRoundRect(362, 68, 116, 78, 8, COR_BOTAO);
    rotulo(408, 103, "USAR");
    tela->drawRoundRect(350, 186, 120, 120, 8, COR_BOTAO);
    rotulo(392, 242, "ATIRAR");
    tela->drawCircle(120, 200, 50, COR_BOTAO);
    rotulo(105, 196, "ANDAR");
  } else {
    tela->drawRoundRect(244, 150, 234, 168, 8, COR_BOTAO);
    rotulo(355, 230, "OK");
    tela->drawCircle(120, 200, 50, COR_BOTAO);
    rotulo(102, 196, "MOVER");
  }
#else
  (void)modo;
#endif
}

void placa_console(const char *texto, int tamanho) {
  Serial.write((const uint8_t *)texto, tamanho);
  if (!consoleAtivo || !tela) return;
  for (int i = 0; i < tamanho; i++) {
    if (tela->getCursorY() > TELA_ALTURA - 10) {
      tela->fillScreen(0x0000);
      tela->setCursor(0, 0);
    }
    tela->write((uint8_t)texto[i]);
  }
  // A tela inteira leva dezenas de ms para ir: não manda a cada letra.
  if (millis() - consoleUltimoEnvio > 150) {
    tela->flush();
    consoleUltimoEnvio = millis();
  }
}

void placa_console_fim(void) { consoleAtivo = false; }

void placa_tela_erro(const char *titulo, const char *msg) {
  consoleAtivo = false;
  tela->fillScreen(0x0000);
  tela->setTextWrap(true);
  tela->setTextColor(0xF800);
  tela->setTextSize(3);
  tela->setCursor(10, 10);
  tela->println(titulo);
  tela->setTextColor(0xFFFF);
  tela->setTextSize(2);
  tela->setCursor(0, 60);
  tela->println(msg);
}

// ---------------------------------------------------------------- toque

int placa_ler_toques(int16_t x[2], int16_t y[2]) {
  // Protocolo do AXS15231B: comando de leitura com o tamanho pedido, depois
  // 2 bytes de cabeçalho e 6 bytes por dedo.
  const int pedir = 2 + 6 * TOQUE_MAX_PONTOS;
  const uint8_t cmd[11] = {0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00,
                           0x00, (uint8_t)pedir, 0x00, 0x00, 0x00};
  uint8_t buf[2 + 6 * 2] = {0};
  Wire.beginTransmission(ENDERECO_TOQUE);
  Wire.write(cmd, sizeof(cmd));
  if (Wire.endTransmission() != 0) return 0;
  if (Wire.requestFrom(ENDERECO_TOQUE, pedir) != pedir) return 0;
  for (int i = 0; i < pedir; i++) buf[i] = Wire.read();

  int n = buf[1];
  if (n == 0 || n > 2) return 0;
  if (n > TOQUE_MAX_PONTOS) n = TOQUE_MAX_PONTOS;
  int achados = 0;
  for (int i = 0; i < n; i++) {
    const uint8_t *p = buf + 2 + 6 * i;
    int px = ((p[0] & 0x0F) << 8) | p[1];  // coordenadas do painel em pé
    int py = ((p[2] & 0x0F) << 8) | p[3];
    if (px >= PAINEL_LARGURA || py >= PAINEL_ALTURA) continue;
#if ROTACAO_TELA == 1
    x[achados] = py;
    y[achados] = PAINEL_LARGURA - 1 - px;
#else
    x[achados] = PAINEL_ALTURA - 1 - py;
    y[achados] = px;
#endif
    achados++;
  }
  return achados;
}

// ---------------------------------------------------------------- arquivos

int placa_wad_na_flash(void) {
  const esp_partition_t *p = esp_partition_find_first(
      ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "wad");
  char magia[4];
  if (!p || esp_partition_read(p, 0, magia, 4) != ESP_OK) return 0;
  return memcmp(magia, "IWAD", 4) == 0;
}

static void procurarJogo() {
  // Cartão primeiro (jogo completo), depois a flash (episódio 1).
  static const char *nomes[] = {
      "/freedoom1.wad", "/freedoom/freedoom1.wad", "/freedoom2.wad",
      "/freedoom/freedoom2.wad", "/doom.wad", "/doom1.wad", "/doom2.wad",
  };
  if (temCartao) {
    for (const char *nome : nomes) {
      if (cartao->exists(nome)) {
        snprintf(caminhoWad, sizeof(caminhoWad), "/sd%s", nome);
        break;
      }
    }
  }
  if (!caminhoWad[0] && placa_wad_na_flash()) {
    // O nome serve para o motor reconhecer o jogo; os dados vêm da partição.
    strcpy(caminhoWad, "/flash/freedoom1.wad");
  }

  // Saves e default.cfg: no cartão se houver, senão na partição `saves`.
  if (temCartao) {
    if (!cartao->exists("/freedoom")) cartao->mkdir("/freedoom");
    strcpy(pastaDados, "/sd/freedoom/");
  } else if (temSaves) {
    strcpy(pastaDados, "/saves/");
  }
}

const char *placa_caminho_wad(void) { return caminhoWad[0] ? caminhoWad : NULL; }

// Chamado pelo motor (m_config.c).
extern "C" const char *DG_PastaDados(void) { return pastaDados; }

// ---------------------------------------------------------------- início

void placa_iniciar(void) {
  pinMode(PINO_LCD_BL, OUTPUT);
  digitalWrite(PINO_LCD_BL, LOW);

  barramento = new Arduino_ESP32QSPI(PINO_LCD_CS, PINO_LCD_SCK, PINO_LCD_D0,
                                     PINO_LCD_D1, PINO_LCD_D2, PINO_LCD_D3);
  lcd = new Arduino_AXS15231B(barramento, GFX_NOT_DEFINED, 0, false,
                              PAINEL_LARGURA, PAINEL_ALTURA);
  tela = new Arduino_Canvas(PAINEL_LARGURA, PAINEL_ALTURA, lcd, 0, 0, 0);
  tela->begin();
  tela->setRotation(ROTACAO_TELA);  // texto e botões já saem deitados
  tela->fillScreen(0x0000);
  tela->setTextColor(0xFFFF);
  tela->setTextSize(1);
  tela->setTextWrap(true);
  tela->flush();
  digitalWrite(PINO_LCD_BL, HIGH);

  Wire.begin(PINO_TOQUE_SDA, PINO_TOQUE_SCL);
  Wire.setClock(400000);

  // Cartão: primeiro o modo SD nativo de 1 bit (CLK, CMD = MOSI, D0 = MISO),
  // com o D3 (= CS) em nível alto. Se não der, tenta o modo SPI.
  const char *modoCartao = "nao encontrado";
  pinMode(PINO_SD_CS, OUTPUT);
  digitalWrite(PINO_SD_CS, HIGH);
  SD_MMC.setPins(PINO_SD_SCK, PINO_SD_MOSI, PINO_SD_MISO);
  if (SD_MMC.begin("/sd", true, false, SDMMC_FREQ_DEFAULT, 8)) {
    temCartao = true;
    cartao = &SD_MMC;
    modoCartao = "ok (SDMMC 1 bit)";
  } else {
    SD_MMC.end();
    // O cartão não desliga quando o chip reinicia (gravar pelo USB, abrir a serial).
    // Se o reset pegou o cartão no meio de uma leitura, ele ignora o primeiro
    // comando. Como no launcher (sd_card.cpp do JC3248W535EN): pulsos de relógio
    // com o cartão solto e novas tentativas mais devagar, com pausas.
    pinMode(PINO_SD_MISO, INPUT_PULLUP);
    pinMode(PINO_SD_MOSI, INPUT_PULLUP);
    spiCartao.begin(PINO_SD_SCK, PINO_SD_MISO, PINO_SD_MOSI, PINO_SD_CS);
    static const uint32_t frequencia[] = {20000000, 4000000, 4000000, 4000000, 4000000};
    static const uint32_t pausaMs[] = {0, 100, 250, 500, 1000};
    for (int i = 0; i < 5 && !temCartao; i++) {
      if (i > 0) {
        SD.end();
        delay(pausaMs[i]);
        digitalWrite(PINO_SD_CS, HIGH);
        spiCartao.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
        for (int k = 0; k < 1100; k++) spiCartao.transfer(0xFF);  // termina um bloco pendente
        spiCartao.endTransaction();
      }
      if (SD.begin(PINO_SD_CS, spiCartao, frequencia[i], "/sd", 8)) {
        temCartao = true;
        cartao = &SD;
        modoCartao = i == 0 ? "ok (SPI)" : "ok (SPI, depois de destravar)";
      }
    }
    if (!temCartao) modoCartao = "nao encontrado (tire o USB e a bateria por uns segundos)";
  }
  if (!temCartao) {
    // Sem cartão: saves na flash (formata na primeira vez).
    temSaves = LittleFS.begin(true, "/saves", 8, "saves");
  }
  procurarJogo();

  char linha[160];
  int n = snprintf(linha, sizeof(linha), "microSD: %s\nJogo: %s\nSaves: %s\n",
                   modoCartao,
                   caminhoWad[0] ? caminhoWad : "nao encontrado",
                   pastaDados[0] ? pastaDados : "sem lugar para salvar");
  placa_console(linha, n);
}

// ---------------------------------------------------------------- som

int placa_som_iniciar(int taxa_hz) {
  i2s_config_t cfg = {};
  cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
  cfg.sample_rate = taxa_hz;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.dma_buf_count = 4;
  cfg.dma_buf_len = 256;
  cfg.tx_desc_auto_clear = true;
  i2s_pin_config_t pinos = {};
  pinos.mck_io_num = I2S_PIN_NO_CHANGE;
  pinos.bck_io_num = PINO_I2S_BCLK;
  pinos.ws_io_num = PINO_I2S_LRCLK;
  pinos.data_out_num = PINO_I2S_DADOS;
  pinos.data_in_num = I2S_PIN_NO_CHANGE;
  return i2s_driver_install(I2S_NUM_0, &cfg, 0, nullptr) == ESP_OK && i2s_set_pin(I2S_NUM_0, &pinos) == ESP_OK;
}

void placa_som_escrever(const int16_t *estereo, int quadros) {
  size_t escrito;
  i2s_write(I2S_NUM_0, estereo, size_t(quadros) * 4, &escrito, portMAX_DELAY);
}

uint32_t placa_millis(void) { return millis(); }

void placa_reiniciar(void) { ESP.restart(); }
