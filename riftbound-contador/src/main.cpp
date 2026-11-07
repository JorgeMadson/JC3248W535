// Contador de pontos Riftbound - JC3248W535 (ESP32-S3 + tela 3.5" 320x480 AXS15231B)
//
// Layout (placa deitada no meio da mesa, em pé no sentido 320x480):
//   metade de cima  -> OPONENTE (desenhada de cabeça pra baixo, pra ele ler)
//   faixa do meio   -> cronômetro (duas cópias, uma virada pra cada jogador)
//   metade de baixo -> EU
//
// Toques:
//   lado esquerdo do seu painel  -> -1 ponto
//   lado direito do seu painel   -> +1 ponto
//   toque rápido no cronômetro   -> inicia / pausa
//   segurar o cronômetro 2s      -> nova partida (zera pontos e tempo)

#include <Arduino.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>

// ---------- Configuração da partida ----------
static const int  PONTOS_VITORIA  = 8;        // Riftbound 1x1: vence quem chega a 8
static const uint32_t TEMPO_RODADA_S = 50 * 60; // tempo da rodada (mude aqui)

// ---------- Ajustes do toque (mude se o toque sair espelhado) ----------
#define TOUCH_SWAP_XY 0
#define TOUCH_FLIP_X  0
#define TOUCH_FLIP_Y  0
#define DEBUG_TOQUE   0  // 1 = mostra um ponto onde você tocou e imprime na serial

// ---------- Pinos da JC3248W535 ----------
#define PIN_LCD_CS   45
#define PIN_LCD_SCK  47
#define PIN_LCD_D0   21
#define PIN_LCD_D1   48
#define PIN_LCD_D2   40
#define PIN_LCD_D3   39
#define PIN_LCD_BL    1
#define PIN_TP_SDA    4
#define PIN_TP_SCL    8
#define TP_ADDR    0x3B

static const int W = 320, H = 480;
static const int PAINEL_H = 205;               // altura de cada painel de jogador
static const int MEIO_Y = PAINEL_H;            // faixa do cronômetro
static const int MEIO_H = H - 2 * PAINEL_H;    // 70 px

Arduino_DataBus *bus = new Arduino_ESP32QSPI(PIN_LCD_CS, PIN_LCD_SCK, PIN_LCD_D0, PIN_LCD_D1, PIN_LCD_D2, PIN_LCD_D3);
Arduino_GFX *lcd = new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, false, W, H);
Arduino_Canvas *gfx = new Arduino_Canvas(W, H, lcd, 0, 0, 0);

// ---------- Cores (RGB565) ----------
static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }
static const uint16_t COR_FUNDO     = rgb(10, 12, 20);
static const uint16_t COR_EU        = rgb(30, 120, 220);
static const uint16_t COR_EU_ESC    = rgb(12, 40, 80);
static const uint16_t COR_OPO       = rgb(220, 60, 60);
static const uint16_t COR_OPO_ESC   = rgb(80, 18, 22);
static const uint16_t COR_OURO      = rgb(240, 190, 60);
static const uint16_t COR_TEXTO     = rgb(240, 240, 245);
static const uint16_t COR_APAGADO   = rgb(120, 125, 140);
static const uint16_t COR_MEIO      = rgb(22, 25, 38);

// ---------- Estado ----------
int pontosEu = 0, pontosOpo = 0;
bool rodando = false;
uint32_t decorridoMs = 0;     // tempo acumulado
uint32_t inicioMs = 0;        // millis() de quando deu play
bool precisaDesenhar = true;
uint32_t ultimoSegundo = 0xFFFFFFFF;
int toqueX = -1, toqueY = -1; uint32_t toqueAte = 0;

uint32_t tempoDecorridoMs() { return decorridoMs + (rodando ? millis() - inicioMs : 0); }

// ---------- Toque (AXS15231B integrado) ----------
bool lerToque(int &x, int &y) {
  static const uint8_t cmd[11] = {0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00};
  uint8_t buf[8] = {0};
  Wire.beginTransmission(TP_ADDR);
  Wire.write(cmd, sizeof(cmd));
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom(TP_ADDR, 8) != 8) return false;
  for (int i = 0; i < 8; i++) buf[i] = Wire.read();
  uint8_t n = buf[1];
  if (n == 0 || n > 2) return false;
  int rx = ((buf[2] & 0x0F) << 8) | buf[3];
  int ry = ((buf[4] & 0x0F) << 8) | buf[5];
#if TOUCH_SWAP_XY
  int t = rx; rx = ry; ry = t;
#endif
#if TOUCH_FLIP_X
  rx = W - 1 - rx;
#endif
#if TOUCH_FLIP_Y
  ry = H - 1 - ry;
#endif
  if (rx < 0 || rx >= W || ry < 0 || ry >= H) return false;
  x = rx; y = ry;
  return true;
}

// ---------- Desenho ----------
void textoCentro(const char *s, int cx, int cy, uint8_t tam, uint16_t cor) {
  int largura = strlen(s) * 6 * tam - tam;  // fonte padrão 6x8
  int altura = 8 * tam;
  gfx->setTextSize(tam);
  gfx->setTextColor(cor);
  gfx->setCursor(cx - largura / 2, cy - altura / 2);
  gfx->print(s);
}

// Desenha um painel de jogador na parte de BAIXO da orientação atual do canvas.
void desenhaPainel(const char *nome, int pontos, uint16_t cor, uint16_t corEsc) {
  int y0 = H - PAINEL_H;
  bool venceu = pontos >= PONTOS_VITORIA;
  gfx->fillRect(0, y0, W, PAINEL_H, venceu ? rgb(60, 45, 10) : corEsc);

  // botões - e +
  int by = y0 + 50, bh = 110, bw = 70;
  gfx->fillRoundRect(10, by, bw, bh, 14, cor);
  gfx->fillRoundRect(W - 10 - bw, by, bw, bh, 14, cor);
  textoCentro("-", 10 + bw / 2, by + bh / 2, 7, COR_TEXTO);
  textoCentro("+", W - 10 - bw / 2, by + bh / 2, 7, COR_TEXTO);

  // placar
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", pontos);
  textoCentro(buf, W / 2, by + bh / 2, pontos >= 10 ? 9 : 12, venceu ? COR_OURO : COR_TEXTO);

  // nome / vitória (perto da borda, do lado de quem lê)
  if (venceu) textoCentro("VENCEU!", W / 2, H - 22, 3, COR_OURO);
  else {
    textoCentro(nome, W / 2, H - 22, 2, COR_APAGADO);
    snprintf(buf, sizeof(buf), "%d/%d", pontos, PONTOS_VITORIA);
    textoCentro(buf, W / 2, y0 + 22, 2, COR_APAGADO);
  }
}

void formataTempo(char *buf, size_t n) {
  uint32_t s = tempoDecorridoMs() / 1000;
  if (s < TEMPO_RODADA_S) {
    uint32_t r = TEMPO_RODADA_S - s;
    snprintf(buf, n, "%02lu:%02lu", (unsigned long)(r / 60), (unsigned long)(r % 60));
  } else {
    uint32_t e = s - TEMPO_RODADA_S;   // acréscimo depois de acabar
    snprintf(buf, n, "+%lu:%02lu", (unsigned long)(e / 60), (unsigned long)(e % 60));
  }
}

// Cronômetro na metade ESQUERDA da faixa do meio (na orientação atual).
void desenhaTempo() {
  char buf[12];
  formataTempo(buf, sizeof(buf));
  bool acabou = tempoDecorridoMs() / 1000 >= TEMPO_RODADA_S;
  uint16_t cor = acabou ? COR_OPO : (rodando ? COR_TEXTO : COR_APAGADO);
  textoCentro(buf, W / 4, MEIO_Y + MEIO_H / 2 - 6, 4, cor);
  textoCentro(rodando ? "toque p/ pausar" : "toque p/ iniciar", W / 4, MEIO_Y + MEIO_H - 10, 1, COR_APAGADO);
}

void desenhaTudo() {
  gfx->setRotation(0);
  gfx->fillScreen(COR_FUNDO);

  // faixa do meio
  gfx->fillRect(0, MEIO_Y, W, MEIO_H, COR_MEIO);
  gfx->drawFastVLine(W / 2, MEIO_Y + 8, MEIO_H - 16, COR_APAGADO);

  // EU: orientação normal (parte de baixo)
  desenhaPainel("EU", pontosEu, COR_EU, COR_EU_ESC);
  desenhaTempo();

  // OPONENTE: canvas girado 180 graus -> "parte de baixo" vira a de cima
  gfx->setRotation(2);
  desenhaPainel("OPONENTE", pontosOpo, COR_OPO, COR_OPO_ESC);
  desenhaTempo();
  gfx->setRotation(0);

#if DEBUG_TOQUE
  if (toqueX >= 0 && millis() < toqueAte) {
    gfx->fillCircle(toqueX, toqueY, 8, COR_TEXTO);
    gfx->drawCircle(toqueX, toqueY, 12, COR_OURO);
  }
#endif

  gfx->flush();
}

// ---------- Ações ----------
void novaPartida() {
  pontosEu = pontosOpo = 0;
  rodando = false;
  decorridoMs = 0;
  precisaDesenhar = true;
}

void alternaTempo() {
  if (rodando) { decorridoMs += millis() - inicioMs; rodando = false; }
  else { inicioMs = millis(); rodando = true; }
  precisaDesenhar = true;
}

void aoTocar(int x, int y) {
  if (y >= H - PAINEL_H) {                   // meu painel
    if (x < W / 2) { if (pontosEu > 0) pontosEu--; } else pontosEu++;
    precisaDesenhar = true;
  } else if (y < PAINEL_H) {                 // painel do oponente (virado)
    // virado 180: o "esquerdo" dele é o lado direito da tela
    if (x >= W / 2) { if (pontosOpo > 0) pontosOpo--; } else pontosOpo++;
    precisaDesenhar = true;
  }
}

// ---------- Setup / loop ----------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nContador Riftbound iniciando...");

  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, HIGH);

  if (!gfx->begin()) {
    Serial.println("ERRO: falha ao iniciar a tela/canvas");
  } else {
    Serial.println("Tela OK");
  }

  Wire.begin(PIN_TP_SDA, PIN_TP_SCL);
  Wire.setClock(400000);

  novaPartida();
}

void loop() {
  static bool estavaTocando = false;
  static uint32_t inicioToque = 0;
  static int tx0 = 0, ty0 = 0;
  static bool resetFeito = false;

  int x, y;
  bool tocando = lerToque(x, y);

  if (tocando && !estavaTocando) {           // começou um toque
    inicioToque = millis(); tx0 = x; ty0 = y; resetFeito = false;
#if DEBUG_TOQUE
    Serial.printf("toque x=%d y=%d\n", x, y);
    toqueX = x; toqueY = y; toqueAte = millis() + 600; precisaDesenhar = true;
#endif
    aoTocar(x, y);
  }

  bool noMeio = ty0 >= MEIO_Y && ty0 < MEIO_Y + MEIO_H;
  if (tocando && noMeio && !resetFeito && millis() - inicioToque > 2000) {
    novaPartida();                            // segurou 2s no cronômetro
    resetFeito = true;
  }
  if (!tocando && estavaTocando && noMeio && !resetFeito) {
    alternaTempo();                           // toque rápido no cronômetro
  }
  estavaTocando = tocando;

  uint32_t seg = tempoDecorridoMs() / 1000;
  if (seg != ultimoSegundo) { ultimoSegundo = seg; precisaDesenhar = true; }
#if DEBUG_TOQUE
  static bool pontoVisivel = false;
  bool vis = toqueX >= 0 && millis() < toqueAte;
  if (vis != pontoVisivel) { pontoVisivel = vis; precisaDesenhar = true; }
#endif

  if (precisaDesenhar) {
    precisaDesenhar = false;
    desenhaTudo();
  }
  delay(15);
}
