// Testes da orientação espelhada e da pilha de telas.
#include <unity.h>

#include "gerenciador_telas.h"
#include "orientacao.h"

using namespace placar;

void setUp() {}
void tearDown() {}

// ---------- Orientação ----------

void test_lado_de_baixo_nao_muda_coordenada() {
  Ponto p = paraLocal(Lado::Baixo, {10, 300});
  TEST_ASSERT_EQUAL(10, p.x);
  TEST_ASSERT_EQUAL(300, p.y);
}

void test_lado_de_cima_gira_180() {
  Ponto p = paraLocal(Lado::Cima, {0, 0});
  TEST_ASSERT_EQUAL(LARGURA_TELA - 1, p.x);
  TEST_ASSERT_EQUAL(ALTURA_TELA - 1, p.y);
  Ponto volta = paraLocal(Lado::Cima, p);  // a transformação é a própria inversa
  TEST_ASSERT_EQUAL(0, volta.x);
  TEST_ASSERT_EQUAL(0, volta.y);
}

void test_cada_metade_pertence_a_um_lado() {
  TEST_ASSERT_EQUAL(Lado::Cima, ladoDoPonto({100, 0}));
  TEST_ASSERT_EQUAL(Lado::Cima, ladoDoPonto({100, ALTURA_TELA / 2 - 1}));
  TEST_ASSERT_EQUAL(Lado::Baixo, ladoDoPonto({100, ALTURA_TELA / 2}));
  // O "esquerdo" do jogador de cima é o lado direito da tela.
  Ponto local = paraLocal(Lado::Cima, {300, 10});
  TEST_ASSERT_TRUE(local.x < LARGURA_TELA / 2);
  TEST_ASSERT_TRUE(local.y >= ALTURA_TELA / 2);  // a metade dele é sempre a de baixo
}

// ---------- Pilha de telas ----------

struct TelaFalsa : Tela {
  int* destruidas;
  bool ehOpaca;
  int eventos = 0, entradas = 0, desenhos = 0;
  TelaFalsa(int* d, bool opaca = true) : destruidas(d), ehOpaca(opaca) {}
  ~TelaFalsa() override { (*destruidas)++; }
  void aoEntrar() override { entradas++; }
  void aoEvento(const Evento&) override { eventos++; }
  void desenhar(Pintor&) override { desenhos++; }
  bool opaca() const override { return ehOpaca; }
};

static Pintor& pintorFalso() { return *reinterpret_cast<Pintor*>(0x1000); }  // nunca usado

void test_operacoes_so_valem_depois_de_aplicar() {
  int d = 0;
  GerenciadorTelas g;
  g.empilhar(std::unique_ptr<Tela>(new TelaFalsa(&d)));
  TEST_ASSERT_EQUAL(0, g.tamanho());
  g.aplicarPendentes();
  TEST_ASSERT_EQUAL(1, g.tamanho());
}

void test_eventos_vao_so_para_o_topo_e_desenho_inclui_tela_sob_dialogo() {
  int d = 0;
  GerenciadorTelas g;
  auto* jogo = new TelaFalsa(&d);
  auto* dialogo = new TelaFalsa(&d, false);
  g.empilhar(std::unique_ptr<Tela>(new TelaFalsa(&d)));  // menu
  g.empilhar(std::unique_ptr<Tela>(jogo));
  g.empilhar(std::unique_ptr<Tela>(dialogo));
  g.aplicarPendentes();
  g.aoEvento(Evento{});
  TEST_ASSERT_EQUAL(0, jogo->eventos);
  TEST_ASSERT_EQUAL(1, dialogo->eventos);
  TEST_ASSERT_TRUE(g.precisaDesenhar());
  g.desenhar(pintorFalso());
  TEST_ASSERT_EQUAL(1, jogo->desenhos);
  TEST_ASSERT_EQUAL(1, dialogo->desenhos);
  TEST_ASSERT_FALSE(g.precisaDesenhar());
  jogo->marcarSuja();
  TEST_ASSERT_TRUE(g.precisaDesenhar());
}

void test_desempilhar_destroi_e_reentra_na_de_baixo() {
  int d = 0;
  GerenciadorTelas g;
  auto* jogo = new TelaFalsa(&d);
  g.empilhar(std::unique_ptr<Tela>(new TelaFalsa(&d)));
  g.empilhar(std::unique_ptr<Tela>(jogo));
  g.aplicarPendentes();
  g.empilhar(std::unique_ptr<Tela>(new TelaFalsa(&d, false)));
  g.aplicarPendentes();
  int entradasAntes = jogo->entradas;
  g.desempilhar();
  g.aplicarPendentes();
  TEST_ASSERT_EQUAL(1, d);
  TEST_ASSERT_EQUAL(entradasAntes + 1, jogo->entradas);
}

void test_voltar_ao_inicio_mantem_so_o_menu() {
  int d = 0;
  GerenciadorTelas g;
  g.empilhar(std::unique_ptr<Tela>(new TelaFalsa(&d)));
  g.empilhar(std::unique_ptr<Tela>(new TelaFalsa(&d)));
  g.empilhar(std::unique_ptr<Tela>(new TelaFalsa(&d, false)));
  g.aplicarPendentes();
  g.voltarAoInicio();
  g.desempilhar();  // nunca remove a tela inicial
  g.aplicarPendentes();
  TEST_ASSERT_EQUAL(1, g.tamanho());
  TEST_ASSERT_EQUAL(2, d);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_lado_de_baixo_nao_muda_coordenada);
  RUN_TEST(test_lado_de_cima_gira_180);
  RUN_TEST(test_cada_metade_pertence_a_um_lado);
  RUN_TEST(test_operacoes_so_valem_depois_de_aplicar);
  RUN_TEST(test_eventos_vao_so_para_o_topo_e_desenho_inclui_tela_sob_dialogo);
  RUN_TEST(test_desempilhar_destroi_e_reentra_na_de_baixo);
  RUN_TEST(test_voltar_ao_inicio_mantem_so_o_menu);
  return UNITY_END();
}
