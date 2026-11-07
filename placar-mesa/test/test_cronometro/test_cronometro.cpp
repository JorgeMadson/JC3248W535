// Testes do cronômetro regressivo com acréscimo.
#include <unity.h>

#include "cronometro.h"

using namespace placar;

void setUp() {}
void tearDown() {}

static const char* texto(const Cronometro& c, uint32_t agora) {
  static char buf[16];
  Cronometro::formatar(c.ler(agora), buf, sizeof(buf));
  return buf;
}

void test_comeca_cheio_e_pausado() {
  Cronometro c(50 * 60);
  TEST_ASSERT_FALSE(c.rodando());
  TEST_ASSERT_EQUAL_STRING("50:00", texto(c, 999999));
}

void test_conta_so_enquanto_roda() {
  Cronometro c(60);
  c.iniciar(1000);
  TEST_ASSERT_EQUAL_STRING("00:50", texto(c, 11000));
  c.pausar(11000);
  TEST_ASSERT_EQUAL_STRING("00:50", texto(c, 99000));  // pausado não anda
  c.alternar(100000);
  TEST_ASSERT_TRUE(c.rodando());
  TEST_ASSERT_EQUAL_STRING("00:45", texto(c, 105000));
}

void test_depois_de_zerar_conta_acrescimo() {
  Cronometro c(60);
  c.iniciar(0);
  TEST_ASSERT_FALSE(c.esgotado(59999));
  TEST_ASSERT_TRUE(c.esgotado(60000));
  TEST_ASSERT_EQUAL_STRING("+0:00", texto(c, 60000));
  TEST_ASSERT_EQUAL_STRING("+1:05", texto(c, 125000));
}

void test_zerar_volta_ao_tempo_cheio_pausado() {
  Cronometro c(60);
  c.iniciar(0);
  c.zerar();
  TEST_ASSERT_FALSE(c.rodando());
  TEST_ASSERT_EQUAL_STRING("01:00", texto(c, 30000));
}

void test_iniciar_duas_vezes_nao_reinicia() {
  Cronometro c(60);
  c.iniciar(0);
  c.iniciar(10000);
  TEST_ASSERT_EQUAL(20000, c.decorridoMs(20000));
}

void test_relogio_que_estoura_32_bits() {
  Cronometro c(60);
  c.iniciar(0xFFFFFF00u);
  TEST_ASSERT_EQUAL(0x200, c.decorridoMs(0x100));  // millis() deu a volta
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_comeca_cheio_e_pausado);
  RUN_TEST(test_conta_so_enquanto_roda);
  RUN_TEST(test_depois_de_zerar_conta_acrescimo);
  RUN_TEST(test_zerar_volta_ao_tempo_cheio_pausado);
  RUN_TEST(test_iniciar_duas_vezes_nao_reinicia);
  RUN_TEST(test_relogio_que_estoura_32_bits);
  return UNITY_END();
}
