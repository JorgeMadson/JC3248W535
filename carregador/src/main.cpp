// Carregador com tela para a JC3248W535.
//
// Quem carrega é o circuito da placa (com o USB e a bateria ligados no P5).
// Este programa só mede a tensão no GPIO5 (divisor 33k/100k) e mostra:
// tensão, carga aproximada, estado, tempo, velocidade de subida e um gráfico.
//
// Proteção: rodando só na bateria e abaixo de 3,05 V, apaga a tela e dorme,
// acordando a cada 5 min para medir de novo (para não descarregar a LiPo demais).
//
// Limitação: a "carga %" vem de uma tabela de LiPo em repouso. Carregando, a
// tensão fica mais alta que em repouso, então a % aparece maior do que é.

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "soc/usb_serial_jtag_reg.h"
#include "esp_sleep.h"

// ---- Hardware ----
static const int PINO_BAT = 5;
static const int PINO_LUZ = 1;
static const float FATOR_DIVISOR = 1.33f;  // do esquema; conferir com multímetro

Arduino_DataBus *bus = new Arduino_ESP32QSPI(45, 47, 21, 48, 40, 39);
Arduino_GFX *lcd = new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, false, 320, 480);
Arduino_Canvas *gfx = new Arduino_Canvas(320, 480, lcd, 0, 0, 0);

// ---- Limites ----
static const float V_CHEIA = 4.15f;        // acima disso e parada = cheia
static const float V_BAIXA = 3.30f;        // aviso de bateria baixa
static const float V_DORMIR = 3.05f;       // só na bateria: desliga para proteger
static const float V_ACORDAR = 3.15f;      // depois de dormir, só volta acima disso
// Sem bateria no P5, com USB, o pino lê ~3,27 V cravado (saída do carregador).
// Medido em 12/11/2026. Uma bateria de verdade nessa tensão estaria subindo.
static const float V_SEM_BAT = 3.268f, TOL_SEM_BAT = 0.03f, VARIACAO_SEM_BAT = 0.008f;
static const uint64_t SONO_US = 5ULL * 60 * 1000000;

// ---- Cores (RGB565) ----
static const uint16_t CINZA = 0x8410, CINZA_ESC = 0x2104, LARANJA = 0xFD20;

// ---------------------------------------------------------------------------

float lerTensao() {
  uint32_t s = 0;
  for (int i = 0; i < 64; i++) { s += analogReadMilliVolts(PINO_BAT); delayMicroseconds(300); }
  return (s / 64.0f) * FATOR_DIVISOR / 1000.0f;
}

// Há um PC no USB? O contador de quadros (SOF) do USB só anda com um host.
// Num carregador de tomada ele não anda, então isso detecta PC, não energia.
bool usbComPC() {
  auto quadro = [] { return REG_GET_FIELD(USB_SERIAL_JTAG_FRAM_NUM_REG, USB_SERIAL_JTAG_SOF_FRAME_INDEX); };
  uint32_t a = quadro();
  delay(3);
  return quadro() != a;
}

// Carga aproximada de uma LiPo em repouso.
int porcentagem(float v) {
  static const float tab[][2] = {
    {4.20f, 100}, {4.10f, 90}, {4.00f, 80}, {3.92f, 70}, {3.87f, 60}, {3.84f, 50},
    {3.80f, 40},  {3.77f, 30}, {3.73f, 20}, {3.69f, 10}, {3.61f, 5},  {3.27f, 0}};
  const int n = sizeof(tab) / sizeof(tab[0]);
  if (v >= tab[0][0]) return 100;
  if (v <= tab[n - 1][0]) return 0;
  for (int i = 1; i < n; i++)
    if (v >= tab[i][0])
      return (int)(tab[i][1] + (v - tab[i][0]) / (tab[i - 1][0] - tab[i][0]) * (tab[i - 1][1] - tab[i][1]));
  return 0;
}

uint16_t corDaTensao(float v) {
  if (v >= 3.85f) return GREEN;
  if (v >= 3.60f) return YELLOW;
  if (v >= V_BAIXA) return LARANJA;
  return RED;
}

void dormir() {
  digitalWrite(PINO_LUZ, LOW);
  Serial.println("Bateria baixa: dormindo 5 min");
  Serial.flush();
  esp_sleep_enable_timer_wakeup(SONO_US);
  esp_deep_sleep_start();
}

// ---- Histórico para o gráfico ----
// 300 pontos. Começa com 1 ponto a cada 6 s (30 min); quando enche, junta
// pares e dobra o intervalo, para o gráfico sempre mostrar a sessão inteira.
static const int MAX_PTS = 300;
float pts[MAX_PTS];
int nPts = 0;
uint32_t intervaloPt = 6, somaPtN = 0;
float somaPt = 0;

void guardarPonto(float v) {
  somaPt += v;
  if (++somaPtN < intervaloPt) return;
  if (nPts == MAX_PTS) {
    for (int i = 0; i < MAX_PTS / 2; i++) pts[i] = (pts[2 * i] + pts[2 * i + 1]) / 2;
    nPts = MAX_PTS / 2;
    intervaloPt *= 2;
  }
  pts[nPts++] = somaPt / somaPtN;
  somaPt = 0;
  somaPtN = 0;
}

// ---- Médias por minuto, para a velocidade de subida ----
static const int MAX_MIN = 64;
float porMinuto[MAX_MIN];
int nMin = 0;
float somaMin = 0;
int somaMinN = 0;

void guardarMinuto(float v) {
  somaMin += v;
  if (++somaMinN < 60) return;
  if (nMin == MAX_MIN) { memmove(porMinuto, porMinuto + 1, sizeof(float) * (MAX_MIN - 1)); nMin--; }
  porMinuto[nMin++] = somaMin / somaMinN;
  somaMin = 0;
  somaMinN = 0;
}

// mV por minuto nos últimos `janela` minutos (NAN se ainda não deu tempo).
float subida(int janela) {
  if (nMin <= janela) return NAN;
  return (porMinuto[nMin - 1] - porMinuto[nMin - 1 - janela]) * 1000.0f / janela;
}

// ---- Estado ----
enum Estado { MEDINDO, CARREGANDO, CHEIA, PAROU, NA_BATERIA, BAIXA, SEM_BATERIA };
const char *nomeEstado[] = {"Medindo...", "Carregando", "Cheia!", "Parou de subir", "Na bateria", "Bateria baixa!", "Sem bateria?"};
const uint16_t corEstado[] = {CINZA, CYAN, GREEN, RED, YELLOW, RED, LARANJA};

// Últimos 60 s, para reconhecer a tensão "sem bateria" (cravada em ~3,27 V).
static const int JAN_SEG = 60;
float ultSeg[JAN_SEG];
int nSeg = 0, iSeg = 0;

void guardarSegundo(float v) {
  ultSeg[iSeg] = v;
  iSeg = (iSeg + 1) % JAN_SEG;
  if (nSeg < JAN_SEG) nSeg++;
}

bool pareceSemBateria() {
  if (nSeg < JAN_SEG) return false;
  float mn = 99, mx = 0;
  for (int i = 0; i < JAN_SEG; i++) { mn = min(mn, ultSeg[i]); mx = max(mx, ultSeg[i]); }
  float meio = (mn + mx) / 2;
  return mx - mn < VARIACAO_SEM_BAT && fabsf(meio - V_SEM_BAT) < TOL_SEM_BAT;
}

float vInicio = 0, vSuave = 0;
uint32_t tCheia = 0;  // segundos até ficar cheia (0 = ainda não)
uint32_t tSessao = 0; // millis()/1000 quando a sessão (esta bateria) começou

uint32_t segundosDaSessao() { return millis() / 1000 - tSessao; }

// Bateria trocada: zera tudo e recomeça com a tensão nova.
void novaSessao(float v) {
  nPts = 0; intervaloPt = 6; somaPt = 0; somaPtN = 0;
  nMin = 0; somaMin = 0; somaMinN = 0;
  nSeg = 0; iSeg = 0;
  tCheia = 0;
  tSessao = millis() / 1000;
  vInicio = vSuave = v;
}
int segundosBaixa = 0;

Estado calcularEstado(float v, bool pc) {
  float s3 = subida(3), s10 = subida(10);
  uint32_t t = max<uint32_t>(segundosDaSessao(), 1);

  if (tCheia) return CHEIA;  // fica marcada até trocar a bateria
  if (pareceSemBateria()) return SEM_BATERIA;
  if (v >= V_CHEIA && !isnan(s10) && s10 < 0.5f) { tCheia = t; return CHEIA; }
  // Sem 3 min de dados ainda não dá para saber se sobe ou desce.
  if (isnan(s3)) return (!pc && v < V_BAIXA) ? BAIXA : MEDINDO;
  if (s3 < -1.0f) return v < V_BAIXA ? BAIXA : NA_BATERIA;
  if (s3 > 0.3f) return CARREGANDO;
  if (!isnan(s10) && s10 < 0.3f && v < V_CHEIA) return PAROU;
  return MEDINDO;
}

String duracao(uint32_t s) {
  char b[24];
  if (s >= 3600) snprintf(b, sizeof b, "%luh%02lum", s / 3600, (s / 60) % 60);
  else snprintf(b, sizeof b, "%lum%02lus", s / 60, s % 60);
  return String(b);
}

// ---- Desenho ----
void textoCentro(const char *t, int y, int tam, uint16_t cor) {
  gfx->setTextSize(tam);
  gfx->setTextColor(cor);
  int w = strlen(t) * 6 * tam;
  gfx->setCursor((320 - w) / 2, y);
  gfx->print(t);
}

void desenhar(float v, int pct, Estado e, bool pc) {
  char b[40];
  uint32_t t = segundosDaSessao();
  uint16_t cor = e == SEM_BATERIA ? CINZA : corDaTensao(v);
  if (e == SEM_BATERIA) pct = 0;
  gfx->fillScreen(BLACK);

  // Cabeçalho
  gfx->setTextSize(3); gfx->setTextColor(WHITE);
  gfx->setCursor(12, 12); gfx->print("Carregador");
  gfx->setTextSize(2); gfx->setTextColor(pc ? GREEN : CINZA_ESC);
  gfx->setCursor(270, 16); gfx->print("PC");

  // Tensão
  snprintf(b, sizeof b, "%.2fV", v);
  textoCentro(b, 58, 8, cor);

  // Barra de carga
  int bx = 30, by = 140, bw = 230, bh = 40;
  gfx->drawRect(bx, by, bw, bh, WHITE);
  gfx->fillRect(bx + bw, by + 12, 8, 16, WHITE);
  gfx->fillRect(bx + 3, by + 3, (bw - 6) * pct / 100, bh - 6, cor);
  snprintf(b, sizeof b, "~%d%%", pct);
  gfx->setTextSize(3);
  gfx->setTextColor(pct > 45 ? BLACK : WHITE);
  gfx->setCursor(bx + (bw - strlen(b) * 18) / 2, by + 9);
  gfx->print(b);

  // Estado
  if (segundosBaixa >= 15) textoCentro("Desligando: ligue o USB", 202, 2, RED);
  else textoCentro(nomeEstado[e], 198, 3, corEstado[e]);

  // Detalhes
  gfx->setTextSize(2); gfx->setTextColor(CINZA);
  float s = subida(5);
  if (isnan(s)) snprintf(b, sizeof b, "subida: aguarde %d min", 6 - nMin);
  else snprintf(b, sizeof b, "subida: %+.1f mV/min", s);
  gfx->setCursor(12, 236); gfx->print(b);
  gfx->setCursor(12, 258);
  if (tCheia) gfx->printf("encheu em %s", duracao(tCheia).c_str());
  else gfx->printf("tempo %s  inicio %.2fV", duracao(t).c_str(), vInicio);

  // Gráfico 3,0 V a 4,3 V
  const int gx = 40, gy = 290, gw = MAX_PTS - 30, gh = 175;
  const float vmin = 3.0f, vmax = 4.3f;
  auto yDe = [&](float x) { return gy + gh - (int)((constrain(x, vmin, vmax) - vmin) / (vmax - vmin) * gh); };
  gfx->setTextSize(1); gfx->setTextColor(CINZA);
  for (float ref : {3.0f, 3.5f, 3.7f, 4.2f}) {
    int y = yDe(ref);
    for (int x = gx; x < gx + gw; x += 4) gfx->drawPixel(x, y, CINZA_ESC);
    gfx->setCursor(8, y - 3); gfx->printf("%.1f", ref);
  }
  gfx->drawRect(gx, gy, gw, gh, CINZA_ESC);
  for (int i = 1; i < nPts; i++) {
    int x0 = gx + (i - 1) * (gw - 1) / (MAX_PTS - 1), x1 = gx + i * (gw - 1) / (MAX_PTS - 1);
    gfx->drawLine(x0, yDe(pts[i - 1]), x1, yDe(pts[i]), corDaTensao(pts[i]));
  }
  gfx->setCursor(gx, gy + gh + 3);
  gfx->printf("ultimos %s", duracao(nPts * intervaloPt).c_str());

  gfx->flush();
}

// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  analogSetPinAttenuation(PINO_BAT, ADC_11db);
  pinMode(PINO_LUZ, OUTPUT);
  digitalWrite(PINO_LUZ, LOW);

  float v = lerTensao();
  // Acordou do sono de proteção: só volta se a bateria subiu (ou se há PC).
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER && v < V_ACORDAR && !usbComPC()) dormir();

  gfx->begin();
  digitalWrite(PINO_LUZ, HIGH);
  novaSessao(v);
}

void loop() {
  static uint32_t proximo = 0;
  if (millis() < proximo) { delay(5); return; }
  proximo = millis() + 1000;

  float v = lerTensao();
  bool pc = usbComPC();
  // Salto brusco sem o USB mudar = bateria trocada. (Tirar o USB também faz a
  // tensão cair um pouco, por isso não conta quando o PC some ou aparece.)
  static bool pcAntes = pc;
  if (fabsf(v - vSuave) > 0.12f && pc == pcAntes) {
    Serial.printf("Salto %.3f -> %.3f V: bateria trocada, nova sessao\n", vSuave, v);
    novaSessao(v);
  }
  pcAntes = pc;
  vSuave += (v - vSuave) * 0.3f;
  guardarPonto(v);
  guardarMinuto(v);
  guardarSegundo(v);

  Estado e = calcularEstado(vSuave, pc);
  int pct = porcentagem(vSuave);
  desenhar(vSuave, pct, e, pc);

  Serial.printf("t=%lus V=%.3f pct=%d pc=%d subida5=%.2f estado=%s\n",
                millis() / 1000, v, pct, pc, subida(5), nomeEstado[e]);

  // Proteção: só na bateria e muito baixa por 30 s seguidos.
  segundosBaixa = (!pc && vSuave < V_DORMIR) ? segundosBaixa + 1 : 0;
  if (segundosBaixa >= 30) dormir();
}
