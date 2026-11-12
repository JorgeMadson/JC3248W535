# Medidas a tirar (paquímetro, 0,1 mm, duas vezes cada)

Referência para posições: placa **de costas**, USB-C à direita (como na pág. 5 do PDF da
Guition), origem no canto **inferior esquerdo**, X para a direita, Y para cima.
Cada medida tem o nome da variável que ela troca em `parametros.scad`.

## A. Placa (para a case base e o teste)
1. Comprimento × largura totais. Confere os 94,5 × 62,0 → `placa_c`, `placa_l`
2. **Espessura total**, da tela às costas (e no degrau, se a borda tiver) → `placa_e`
3. Furos dos cantos: diâmetro, rosca (testar M2 / M2,5 / M3), distância da borda até a
   borda do furo em X e em Y, profundidade → `furo_mx`, `furo_my`, `parafuso_*`
4. Raio dos cantos → `placa_r`
5. Moldura da frente: da borda até a área acesa da tela, nos 4 lados → `tela_dx`, `tela_dy`
6. USB-C: posição do centro, tamanho da abertura e a **capa do plugue do seu cabo**
   (largura × altura × comprimento) → linha "USB-C" de `conectores`
7. microSD: em que borda fica, posição, altura medida das costas, quanto o cartão sobra para fora
8. P1–P6, botão da bateria, Boot/Reset: X/Y do centro e tamanho → `conectores`
9. A peça mais alta que sai das costas (mm) → `folga_tras`

## B. Baterias (cada uma)
10. C × L × E reais (LiPo incha) → `baterias`
11. Saída e comprimento dos fios, e o conector (tamanho e passo = distância do 1º ao último
    pino ÷ (pinos − 1)) → `baia_c`

## C. Alto-falante e capa da Go 3
12. Alto-falante: aro externo, profundidade total, abas/furos de fixação, posição dos terminais
13. Capa: C × L × profundidade **por dentro**, espessura da parede e do lábio, posição e
    tamanho de cada abertura (botões, USB, alça). Fotos por dentro

## D. NRF24 PA+LNA
14. Placa C × L × E, furos, altura dos pinos, posição e altura do SMA; antena: diâmetro e
    comprimento → `nrf_*`, `sma_z`
