# Prompt: evoluir o contador para um "placar de mesa" com vários jogos

> Cole tudo abaixo da linha em um novo chat do Claude Code aberto na pasta
> `C:\Users\filho\dev\JC3248W535`.

---

Você vai evoluir o firmware de uma placa **JC3248W535** (ESP32-S3, tela touch 3.5"
320×480, controlador AXS15231B via QSPI). Hoje ela roda um contador de pontos de
**Riftbound** feito como protótipo num único arquivo. Quero transformá-lo num
**placar de mesa com vários jogos**, bem arquitetado, para que eu adicione jogos e
opções novas no futuro mexendo o mínimo possível no código existente.

## Contexto — leia antes de qualquer coisa

1. `README.md`: tudo sobre a placa (pinos, chip, protocolo do toque, biblioteca
   que funciona, problemas já resolvidos, como restaurar o sistema de fábrica).
2. `riftbound-contador/platformio.ini` e `riftbound-contador/src/main.cpp`: o
   protótipo atual, que **funciona** (tela e toque validados na placa). Use-o como
   referência de inicialização de hardware: pinos, `Arduino_ESP32QSPI` +
   `Arduino_AXS15231B` + `Arduino_Canvas` com `flush()`, leitura I2C do toque em
   `0x3B` e `setRotation(2)` no canvas para desenhar a metade do oponente de
   cabeça pra baixo.

Restrições que já descobrimos (não mude sem motivo):
- PlatformIO, `espressif32@6.9.0`, `GFX Library for Arduino@1.4.9`.
- Manter `build_dir`/`libdeps_dir` em `C:/Users/filho/.rb/`, por causa do limite de
  260 caracteres de caminho do Windows.
- A placa fica na **COM3**. Se o esptool não conectar, peça para eu reconectar o cabo.
- O Python está em pasta protegida: use `python -m platformio` / `python -m esptool`.
- **Nunca** apague nem sobrescreva `backup/`.

## Uso real

A placa fica **deitada no meio da mesa**, entre dois jogadores frente a frente.
Cada metade da tela é virada para um jogador. Tudo que um jogador precisa ler ou
tocar tem que estar na orientação dele.

## O que quero

### 1. Tela inicial: escolher o jogo
- Ao ligar, mostrar um menu com os jogos disponíveis (por enquanto **Riftbound** e
  **Magic: The Gathering**), com botões grandes.
- O menu é gerado a partir da lista de jogos registrados. Adicionar um jogo novo
  **não** pode exigir editar o menu.
- De dentro de qualquer jogo, ter um jeito de voltar ao menu (com confirmação,
  para não sair sem querer).

### 2. Riftbound (melhorar o que existe)
- **Melhor de três**: mostrar quantas partidas cada um venceu (ex.: bolinhas ou
  "1–0"). Quando alguém chega a 8 pontos, a partida é dele: mostrar o resultado e
  oferecer "próxima partida", que zera os pontos e mantém o placar da série.
  Quem vence 2 partidas leva a série: mostrar o vencedor e oferecer "nova série".
- **Fonte dos pontos melhor**: trocar a fonte bitmap 6×8 ampliada (que fica
  serrilhada) por uma fonte grande e lisa. A `GFX Library for Arduino` suporta
  fontes u8g2 (ex.: `u8g2_font_logisoso92_tn` para números) e fontes no formato
  Adafruit GFX. Escolha uma que fique bonita e legível a ~1 m de distância e
  aplique o mesmo padrão de tipografia em todas as telas.
- **Opção de resetar**: botões/menu para (a) zerar só os pontos, (b) zerar só o
  tempo, (c) nova série completa. Sempre com confirmação. Hoje o reset é
  "segurar 2 s no cronômetro", que ninguém descobre sozinho: deixe visível.
- Manter: vitória com 8 pontos, cronômetro regressivo (padrão 50 min) que conta o
  acréscimo depois de zerar, tempo visível para os dois lados, toque rápido no
  tempo para iniciar/pausar.

### 3. Magic: The Gathering (novo)
- Contador de **vida** para 2 jogadores, começando em **20**.
- +1/−1 com toque; segurar o botão repete rápido, ou ter também botões ±5.
- Mostrar a variação recente (ex.: "−7" que some depois de uns segundos), como os
  apps de Magic fazem.
- Opcional, se couber bem na arquitetura: contador de veneno e escolha da vida
  inicial (20 / 30 / 40 para Commander).
- Cronômetro igual ao do Riftbound (reaproveitado, não copiado).
- Reset de vida e de tempo com confirmação.

## Como quero que seja feito (nível engenheiro sênior)

Antes de codar, **proponha a arquitetura** (camadas, interfaces principais, layout
de pastas) e espere meu ok. Diretrizes:

- **Separar camadas**:
  - *HAL / drivers*: tela (canvas + flush), toque (leitura I2C → eventos) e
    backlight. Nada de pino ou I2C fora daqui.
  - *Entrada*: transformar leituras cruas em eventos (`Tap`, `LongPress`,
    `Repeat`, `Release`) com debounce, num só lugar.
  - *UI*: componentes reutilizáveis (botão, placar grande, cronômetro, diálogo de
    confirmação, painel de jogador espelhado), sem regras de jogo.
  - *Lógica dos jogos*: estado e regras puros (sem desenhar, sem `millis()`
    direto: injete o relógio), para poderem ser testados no PC.
- **Jogos como plugins**: uma interface (ex.: `class Game` / `Screen` com
  `onEnter`, `onEvent`, `update`, `draw`, `name`) e um registro de jogos.
  Adicionar um jogo = criar um arquivo e registrá-lo em **um** lugar.
- **Navegação** por uma pilha/gerenciador de telas (menu → jogo → diálogo), em vez
  de `if`s espalhados.
- **Layout espelhado** resolvido uma vez: um helper que desenha e testa o toque
  "na orientação do jogador N", para que cada jogo não precise pensar em rotação
  nem em inverter coordenadas.
- **Configuração** de cada jogo (pontos para vencer, vida inicial, tempo da
  rodada, cores) em structs/constantes claras, não números mágicos no meio do
  código.
- Redesenhar só quando algo muda (dirty flag), mantendo o `flush()` de tela inteira,
  que funciona bem nesta tela.
- **Testes**: um ambiente `[env:native]` no PlatformIO com Unity, cobrindo a lógica
  dos jogos (melhor de três, limites de pontos/vida, cronômetro e acréscimo,
  resets) e o reconhecedor de gestos. Os testes devem passar com
  `python -m platformio test -e native`.
- Código e comentários em português, nomes consistentes, arquivos pequenos e com
  uma responsabilidade cada.
- Opcional, se for simples: guardar a última escolha de jogo e configurações em
  NVS (`Preferences`).

## Processo e verificação

1. Leia o contexto, proponha a arquitetura e espere meu ok.
2. Implemente em etapas pequenas; compile a cada etapa
   (`python -m platformio run`).
3. Rode os testes nativos.
4. Grave na placa (`python -m platformio run -t upload`) e confira pela serial
   que iniciou sem erro.
5. **Você não consegue ver a tela**: no fim de cada etapa visual, me diga
   exatamente o que devo ver e testar (quais toques, o que deve acontecer) e espere
   minha confirmação antes de seguir. Deixe um modo de debug que desenhe um ponto
   onde o toque foi detectado e que possa ser ligado com uma constante.
6. No final, atualize o `README.md` com a nova estrutura e um passo a passo
   curto de "como adicionar um jogo novo".
7. Se quiser, mantenha o protótipo antigo como referência, mas o projeto novo deve
   ficar numa pasta própria (ex.: `placar-mesa/`).
