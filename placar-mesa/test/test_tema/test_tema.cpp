// Testes da mistura de cores (tema.h), usada no brilho e nos contornos do visual Holofote.
#include <unity.h>

#include "../../src/ui/tema.h"

using namespace placar;

void setUp() {}
void tearDown() {}

void test_pontas_dao_as_proprias_cores() {
  uint16_t a = rgb(14, 15, 21), b = rgb(41, 112, 245);
  TEST_ASSERT_EQUAL_HEX16(a, misturar(a, b, 0));
  TEST_ASSERT_EQUAL_HEX16(b, misturar(a, b, 256));
}

void test_meio_fica_entre_as_duas_em_cada_canal() {
  uint16_t m = misturar(rgb(0, 0, 0), rgb(248, 252, 248), 128);  // branco 565 cheio
  TEST_ASSERT_EQUAL_UINT8(15, (m >> 11) & 0x1F);
  TEST_ASSERT_EQUAL_UINT8(31, (m >> 5) & 0x3F);
  TEST_ASSERT_EQUAL_UINT8(15, m & 0x1F);
}

void test_canal_que_diminui_nao_vaza_para_o_vizinho() {
  // vermelho -> azul: o vermelho cai, o azul sobe, o verde fica zero
  uint16_t m = misturar(rgb(248, 0, 0), rgb(0, 0, 248), 64);
  TEST_ASSERT_EQUAL_UINT8(24, (m >> 11) & 0x1F);
  TEST_ASSERT_EQUAL_UINT8(0, (m >> 5) & 0x3F);
  TEST_ASSERT_EQUAL_UINT8(7, m & 0x1F);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_pontas_dao_as_proprias_cores);
  RUN_TEST(test_meio_fica_entre_as_duas_em_cada_canal);
  RUN_TEST(test_canal_que_diminui_nao_vaza_para_o_vizinho);
  return UNITY_END();
}
