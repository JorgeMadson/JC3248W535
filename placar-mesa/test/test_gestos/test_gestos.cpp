// Testes do reconhecedor de gestos (toque cru -> eventos).
#include <unity.h>

#include "gestos.h"

using namespace placar;

static ReconhecedorGestos* g;
static Evento ev[ReconhecedorGestos::MAX_EVENTOS];

void setUp() { g = new ReconhecedorGestos(ConfigGestos{500, 120, 40}); }
void tearDown() { delete g; }

static int passo(bool tocando, uint32_t t, int16_t x = 10, int16_t y = 20) {
  return g->processar(tocando, Ponto{x, y}, t, ev);
}

void test_toque_curto_gera_tap_e_release_ao_soltar() {
  TEST_ASSERT_EQUAL(0, passo(true, 0));
  TEST_ASSERT_EQUAL(0, passo(true, 100));
  TEST_ASSERT_EQUAL(0, passo(false, 120));   // ainda dentro da tolerância
  TEST_ASSERT_EQUAL(2, passo(false, 140));   // 40 ms sem contato: soltou
  TEST_ASSERT_EQUAL(TipoEvento::Tap, ev[0].tipo);
  TEST_ASSERT_EQUAL(TipoEvento::Release, ev[1].tipo);
  TEST_ASSERT_EQUAL(10, ev[0].ponto.x);
  TEST_ASSERT_EQUAL(20, ev[0].ponto.y);
}

void test_leitura_perdida_nao_vira_dois_toques() {
  passo(true, 0);
  TEST_ASSERT_EQUAL(0, passo(false, 20));   // falha de leitura
  TEST_ASSERT_EQUAL(0, passo(true, 40));    // voltou: mesmo toque
  passo(false, 60);
  TEST_ASSERT_EQUAL(2, passo(false, 80));
  TEST_ASSERT_EQUAL(TipoEvento::Tap, ev[0].tipo);
}

void test_ponto_do_evento_eh_onde_o_toque_comecou() {
  passo(true, 0, 100, 200);
  passo(true, 50, 130, 260);
  passo(false, 60);
  passo(false, 100);
  TEST_ASSERT_EQUAL(100, ev[0].ponto.x);
  TEST_ASSERT_EQUAL(200, ev[0].ponto.y);
}

void test_segurar_gera_longpress_repeticoes_e_release_sem_tap() {
  passo(true, 0);
  TEST_ASSERT_EQUAL(0, passo(true, 499));
  TEST_ASSERT_EQUAL(1, passo(true, 500));
  TEST_ASSERT_EQUAL(TipoEvento::LongPress, ev[0].tipo);
  TEST_ASSERT_EQUAL(0, passo(true, 600));
  TEST_ASSERT_EQUAL(1, passo(true, 620));
  TEST_ASSERT_EQUAL(TipoEvento::Repeat, ev[0].tipo);
  TEST_ASSERT_EQUAL(0, passo(true, 700));
  TEST_ASSERT_EQUAL(1, passo(true, 740));
  TEST_ASSERT_EQUAL(TipoEvento::Repeat, ev[0].tipo);
  passo(false, 750);
  TEST_ASSERT_EQUAL(1, passo(false, 790));
  TEST_ASSERT_EQUAL(TipoEvento::Release, ev[0].tipo);
}

void test_loop_atrasado_gera_so_uma_repeticao_por_ciclo() {
  passo(true, 0);
  passo(true, 500);
  TEST_ASSERT_EQUAL(1, passo(true, 2000));
  TEST_ASSERT_EQUAL(TipoEvento::Repeat, ev[0].tipo);
}

void test_sem_toque_nao_gera_nada() {
  TEST_ASSERT_EQUAL(0, passo(false, 0));
  TEST_ASSERT_EQUAL(0, passo(false, 1000));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_toque_curto_gera_tap_e_release_ao_soltar);
  RUN_TEST(test_leitura_perdida_nao_vira_dois_toques);
  RUN_TEST(test_ponto_do_evento_eh_onde_o_toque_comecou);
  RUN_TEST(test_segurar_gera_longpress_repeticoes_e_release_sem_tap);
  RUN_TEST(test_loop_atrasado_gera_so_uma_repeticao_por_ciclo);
  RUN_TEST(test_sem_toque_nao_gera_nada);
  return UNITY_END();
}
