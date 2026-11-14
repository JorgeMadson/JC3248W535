// Testes das regras do Magic (vida, veneno, variação recente, melhor de três).
#include <unity.h>

#include "magic_regras.h"

using namespace placar;
using Estado = MagicRegras::Estado;

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
  MagicRegras n;
  n.mudarVida(0, -23, 0);
  TEST_ASSERT_EQUAL(-3, n.vida(0));
  TEST_ASSERT_TRUE(n.perdeu(0));
}

void test_vida_zero_encerra_a_partida_e_trava() {
  MagicRegras m;
  m.mudarVida(0, -20, 0);
  TEST_ASSERT_EQUAL(Estado::PartidaEncerrada, m.estado());
  TEST_ASSERT_EQUAL(1, m.vencedor());
  TEST_ASSERT_EQUAL(1, m.vitorias(1));
  m.mudarVida(0, +5, 0);  // ignorado: partida encerrada
  m.mudarVida(1, -5, 0);
  m.mudarVeneno(1, +1);
  TEST_ASSERT_EQUAL(0, m.vida(0));
  TEST_ASSERT_EQUAL(20, m.vida(1));
  TEST_ASSERT_EQUAL(0, m.veneno(1));
}

void test_proxima_partida_zera_vida_e_mantem_serie() {
  MagicRegras m;
  m.mudarVida(1, -4, 0);
  m.mudarVida(0, -20, 0);
  m.proximaPartida();
  TEST_ASSERT_EQUAL(Estado::EmJogo, m.estado());
  TEST_ASSERT_EQUAL(20, m.vida(0));
  TEST_ASSERT_EQUAL(20, m.vida(1));
  TEST_ASSERT_EQUAL(1, m.vitorias(1));
  TEST_ASSERT_EQUAL(-1, m.vencedor());
}

void test_segunda_vitoria_encerra_a_serie() {
  MagicRegras m;
  m.mudarVida(0, -20, 0);
  m.proximaPartida();
  m.mudarVida(1, -20, 0);
  m.proximaPartida();
  TEST_ASSERT_EQUAL(Estado::EmJogo, m.estado());
  for (int i = 0; i < 10; i++) m.mudarVeneno(0, +1);  // veneno também encerra
  TEST_ASSERT_EQUAL(Estado::SerieEncerrada, m.estado());
  TEST_ASSERT_EQUAL(1, m.vencedor());
  TEST_ASSERT_EQUAL(2, m.vitorias(1));
  m.proximaPartida();  // não vale com a série encerrada
  TEST_ASSERT_EQUAL(Estado::SerieEncerrada, m.estado());
  m.novaSerie();
  TEST_ASSERT_EQUAL(Estado::EmJogo, m.estado());
  TEST_ASSERT_EQUAL(0, m.vitorias(0));
  TEST_ASSERT_EQUAL(0, m.vitorias(1));
  TEST_ASSERT_EQUAL(0, m.veneno(0));
}

void test_desfazer_derrota_volta_ao_valor_de_antes() {
  MagicRegras m;
  m.mudarVida(0, -17, 0);
  m.mudarVida(0, -5, 0);  // 3 -> -2: perdeu
  TEST_ASSERT_EQUAL(Estado::PartidaEncerrada, m.estado());
  m.desfazerDerrota();
  TEST_ASSERT_EQUAL(Estado::EmJogo, m.estado());
  TEST_ASSERT_EQUAL(3, m.vida(0));
  TEST_ASSERT_EQUAL(0, m.vitorias(1));
  TEST_ASSERT_EQUAL(-1, m.vencedor());

  for (int i = 0; i < 10; i++) m.mudarVeneno(1, +1);
  TEST_ASSERT_EQUAL(0, m.vencedor());
  m.desfazerDerrota();
  TEST_ASSERT_EQUAL(9, m.veneno(1));
  TEST_ASSERT_EQUAL(0, m.vitorias(0));
}

void test_zerar_mantem_a_serie_e_vida_inicial_recomeca() {
  MagicRegras m;
  m.mudarVida(0, -20, 0);
  m.proximaPartida();
  m.mudarVida(1, -6, 0);
  m.zerar();
  TEST_ASSERT_EQUAL(20, m.vida(1));
  TEST_ASSERT_EQUAL(1, m.vitorias(1));
  m.definirVidaInicial(30);
  TEST_ASSERT_EQUAL(0, m.vitorias(1));
  TEST_ASSERT_EQUAL(30, m.vida(0));
}

void test_limites_de_vida() {
  MagicRegras m;
  m.mudarVida(1, +5000, 0);
  TEST_ASSERT_EQUAL(m.config().vidaMax, m.vida(1));
  m.mudarVida(0, -1000, 0);
  TEST_ASSERT_EQUAL(m.config().vidaMin, m.vida(0));
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
  RUN_TEST(test_vida_zero_encerra_a_partida_e_trava);
  RUN_TEST(test_proxima_partida_zera_vida_e_mantem_serie);
  RUN_TEST(test_segunda_vitoria_encerra_a_serie);
  RUN_TEST(test_desfazer_derrota_volta_ao_valor_de_antes);
  RUN_TEST(test_zerar_mantem_a_serie_e_vida_inicial_recomeca);
  return UNITY_END();
}
