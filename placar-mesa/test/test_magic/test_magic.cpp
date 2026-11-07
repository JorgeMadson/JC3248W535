// Testes das regras do Magic (vida, veneno, variação recente).
#include <unity.h>

#include "magic_regras.h"

using namespace placar;

void setUp() {}
void tearDown() {}

void test_comeca_com_20_e_sem_veneno() {
  MagicRegras m;
  TEST_ASSERT_EQUAL(20, m.vida(0));
  TEST_ASSERT_EQUAL(20, m.vida(1));
  TEST_ASSERT_EQUAL(0, m.veneno(0));
  TEST_ASSERT_FALSE(m.perdeu(0));
}

void test_variacao_acumula_e_some_depois_da_janela() {
  MagicRegras m;  // janela padrão 3000 ms
  m.mudarVida(0, -1, 1000);
  m.mudarVida(0, -5, 1500);
  m.mudarVida(0, -1, 2000);
  TEST_ASSERT_EQUAL(13, m.vida(0));
  TEST_ASSERT_EQUAL(-7, m.variacaoVisivel(0, 4999));
  TEST_ASSERT_EQUAL(0, m.variacaoVisivel(0, 5000));
  TEST_ASSERT_EQUAL(0, m.variacaoVisivel(1, 2000));  // o outro não mexeu
}

void test_variacao_recomeca_depois_que_sumiu() {
  MagicRegras m;
  m.mudarVida(1, +3, 0);
  m.mudarVida(1, -2, 10000);
  TEST_ASSERT_EQUAL(-2, m.variacaoVisivel(1, 10000));
}

void test_vida_zero_ou_negativa_perde() {
  MagicRegras m;
  m.mudarVida(0, -20, 0);
  TEST_ASSERT_TRUE(m.perdeu(0));
  m.mudarVida(0, -3, 0);
  TEST_ASSERT_EQUAL(-3, m.vida(0));
}

void test_limites_de_vida() {
  MagicRegras m;
  m.mudarVida(0, -1000, 0);
  TEST_ASSERT_EQUAL(m.config().vidaMin, m.vida(0));
  m.mudarVida(1, +5000, 0);
  TEST_ASSERT_EQUAL(m.config().vidaMax, m.vida(1));
}

void test_dez_venenos_perde_e_nao_fica_negativo() {
  MagicRegras m;
  m.mudarVeneno(1, -1);
  TEST_ASSERT_EQUAL(0, m.veneno(1));
  for (int i = 0; i < 10; i++) m.mudarVeneno(1, +1);
  TEST_ASSERT_TRUE(m.perdeu(1));
}

void test_zerar_e_vida_inicial_commander() {
  MagicRegras m;
  m.mudarVida(0, -4, 0);
  m.mudarVeneno(0, 2);
  m.definirVidaInicial(40);
  TEST_ASSERT_EQUAL(40, m.vida(0));
  TEST_ASSERT_EQUAL(40, m.vida(1));
  TEST_ASSERT_EQUAL(0, m.veneno(0));
  TEST_ASSERT_EQUAL(0, m.variacaoVisivel(0, 0));
  m.mudarVida(1, -10, 0);
  m.zerar();
  TEST_ASSERT_EQUAL(40, m.vida(1));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_comeca_com_20_e_sem_veneno);
  RUN_TEST(test_variacao_acumula_e_some_depois_da_janela);
  RUN_TEST(test_variacao_recomeca_depois_que_sumiu);
  RUN_TEST(test_vida_zero_ou_negativa_perde);
  RUN_TEST(test_limites_de_vida);
  RUN_TEST(test_dez_venenos_perde_e_nao_fica_negativo);
  RUN_TEST(test_zerar_e_vida_inicial_commander);
  return UNITY_END();
}
