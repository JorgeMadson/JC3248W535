// Testes das regras do Riftbound (pontos, partida até 8, melhor de três).
#include <unity.h>

#include "riftbound_regras.h"

using namespace placar;
using Estado = RiftboundRegras::Estado;

void setUp() {}
void tearDown() {}

static void fazerPontos(RiftboundRegras& r, int j, int n) {
  for (int i = 0; i < n; i++) r.somar(j, +1);
}

void test_pontos_nao_ficam_negativos() {
  RiftboundRegras r;
  r.somar(0, -1);
  TEST_ASSERT_EQUAL(0, r.pontos(0));
}

void test_chegar_a_8_encerra_a_partida() {
  RiftboundRegras r;
  fazerPontos(r, 1, 7);
  TEST_ASSERT_EQUAL(Estado::EmJogo, r.estado());
  r.somar(1, +1);
  TEST_ASSERT_EQUAL(Estado::PartidaEncerrada, r.estado());
  TEST_ASSERT_EQUAL(1, r.vencedor());
  TEST_ASSERT_EQUAL(1, r.vitorias(1));
}

void test_pontos_ignorados_com_partida_encerrada() {
  RiftboundRegras r;
  fazerPontos(r, 0, 8);
  r.somar(1, +1);
  r.somar(0, -3);
  TEST_ASSERT_EQUAL(0, r.pontos(1));
  TEST_ASSERT_EQUAL(8, r.pontos(0));
}

void test_proxima_partida_zera_pontos_e_mantem_serie() {
  RiftboundRegras r;
  fazerPontos(r, 1, 3);
  fazerPontos(r, 0, 8);
  r.proximaPartida();
  TEST_ASSERT_EQUAL(Estado::EmJogo, r.estado());
  TEST_ASSERT_EQUAL(0, r.pontos(0));
  TEST_ASSERT_EQUAL(0, r.pontos(1));
  TEST_ASSERT_EQUAL(1, r.vitorias(0));
  TEST_ASSERT_EQUAL(-1, r.vencedor());
}

void test_segunda_vitoria_encerra_a_serie() {
  RiftboundRegras r;
  fazerPontos(r, 0, 8);
  r.proximaPartida();
  fazerPontos(r, 1, 8);
  r.proximaPartida();
  TEST_ASSERT_EQUAL(Estado::EmJogo, r.estado());
  fazerPontos(r, 0, 8);
  TEST_ASSERT_EQUAL(Estado::SerieEncerrada, r.estado());
  TEST_ASSERT_EQUAL(0, r.vencedor());
  r.proximaPartida();  // não vale com a série encerrada
  TEST_ASSERT_EQUAL(Estado::SerieEncerrada, r.estado());
}

void test_nova_serie_zera_tudo() {
  RiftboundRegras r;
  fazerPontos(r, 0, 8);
  r.proximaPartida();
  fazerPontos(r, 0, 8);
  r.novaSerie();
  TEST_ASSERT_EQUAL(Estado::EmJogo, r.estado());
  TEST_ASSERT_EQUAL(0, r.vitorias(0));
  TEST_ASSERT_EQUAL(0, r.pontos(0));
}

void test_zerar_pontos_mantem_serie() {
  RiftboundRegras r;
  fazerPontos(r, 0, 8);
  r.proximaPartida();
  fazerPontos(r, 1, 5);
  r.zerarPontos();
  TEST_ASSERT_EQUAL(0, r.pontos(1));
  TEST_ASSERT_EQUAL(1, r.vitorias(0));
}

void test_desfazer_vitoria_volta_para_7() {
  RiftboundRegras r;
  fazerPontos(r, 0, 8);
  r.desfazerVitoria();
  TEST_ASSERT_EQUAL(Estado::EmJogo, r.estado());
  TEST_ASSERT_EQUAL(7, r.pontos(0));
  TEST_ASSERT_EQUAL(0, r.vitorias(0));
}

void test_config_personalizada() {
  ConfigRiftbound cfg;
  cfg.pontosVitoria = 3;
  cfg.partidasParaSerie = 1;
  RiftboundRegras r(cfg);
  fazerPontos(r, 1, 3);
  TEST_ASSERT_EQUAL(Estado::SerieEncerrada, r.estado());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_pontos_nao_ficam_negativos);
  RUN_TEST(test_chegar_a_8_encerra_a_partida);
  RUN_TEST(test_pontos_ignorados_com_partida_encerrada);
  RUN_TEST(test_proxima_partida_zera_pontos_e_mantem_serie);
  RUN_TEST(test_segunda_vitoria_encerra_a_serie);
  RUN_TEST(test_nova_serie_zera_tudo);
  RUN_TEST(test_zerar_pontos_mantem_serie);
  RUN_TEST(test_desfazer_vitoria_volta_para_7);
  RUN_TEST(test_config_personalizada);
  return UNITY_END();
}
