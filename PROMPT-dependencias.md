# Prompt: atualizar as dependências dos firmwares Arduino

> Cole tudo abaixo da linha num chat novo do Claude Code aberto na pasta `~/dev` (Steam Deck),
> com os dois repositórios lado a lado: `JC3248W535/` e `JC3248W535EN-fork/`.

---

Quero atualizar as dependências dos firmwares Arduino da placa **JC3248W535** (ESP32-S3,
tela 3.5" AXS15231B via QSPI, toque I2C no mesmo chip, I2S no P6, microSD). As versões novas
devem ser mais rápidas e ter correções, mas **nada pode quebrar**: cada firmware tem que
continuar funcionando na placa.

## Leia antes

1. `JC3248W535/README.md` e `JC3248W535/INVENTARIO.md`: pinos, protocolo do toque, problemas
   já resolvidos e como voltar ao sistema de fábrica.
2. A memória do projeto (Claude Code), principalmente: testes nativos rodam no distrobox
   `dev-gcc`, e há duas pastas do PlatformIO no Deck.

## Situação atual

| Projeto | Plataforma | Bibliotecas |
|---|---|---|
| `JC3248W535/placar-mesa` | `espressif32@6.9.0` (Arduino 2.0.17 / ESP-IDF 4.4, GCC 8) | GFX Library for Arduino 1.4.9, U8g2 2.36.18 |
| `JC3248W535/freedoom` | idem | GFX 1.4.9; driver de I2S **antigo** (`driver/i2s.h`) em `src/placa.cpp` |
| `JC3248W535/carregador` | idem | GFX 1.4.9 |
| `JC3248W535EN-fork` (launcher) | pioarduino 61.x, **ESP-IDF 6.1** (GCC 15) | LVGL 8.3 (fixo de propósito), ArduinoJson 7, JPEGDEC, PNGdec |
| CI (`.github/workflows/deploy-pages.yml`) | PlatformIO fixo em versão, actions fixadas por SHA | — |
| Gravador (`site/vendor/esp-web-tools`) | esp-web-tools 10.4.0 embutido | — |

Problema concreto: as duas plataformas pedem versões diferentes do `tool-esptoolpy` e não
convivem na mesma `~/.platformio`. Hoje o fork compila com
`PLATFORMIO_CORE_DIR=~/.platformio-idf`.

## O que quero

1. **Levantamento primeiro, sem mudar nada**: para cada dependência, a versão atual, a mais
   nova estável (com data) e o que muda (changelog). Entregue uma tabela e uma recomendação, e
   **espere meu ok**. Use só versões publicadas há pelo menos duas semanas.
2. Caminho que eu acho mais promissor (confirme ou conteste): passar os três projetos Arduino
   para a **pioarduino com Arduino 3.x** (ESP-IDF 5.x), a mesma família de plataforma do fork.
   Isso acaba com as duas pastas do PlatformIO. Pontos que já sei que mudam:
   - I2S: `driver/i2s.h` não existe mais → `ESP_I2S.h` do Arduino 3 ou `driver/i2s_std.h`
     (veja como o fork faz em `src/audio_player.cpp`).
   - LEDC/backlight, `SD`/`SD_MMC`, `LittleFS.begin` e `esp_partition_mmap` mudaram de
     assinatura ou de comportamento no Arduino 3: confira cada uso.
   - GFX Library for Arduino 1.5+/1.6: confira se `Arduino_ESP32QSPI` + `Arduino_AXS15231B` +
     `Arduino_Canvas` continuam com os mesmos parâmetros; a tela **só funciona** com essa
     combinação (TFT_eSPI não serve).
   - GCC novo e C23: o motor do Doom (`freedoom/lib/doomgeneric`) precisa de `-std=gnu11`
     (em C23 `true`/`false` são palavras reservadas). O fork já faz isso no `src/CMakeLists.txt`.
3. **Um projeto por vez**, nesta ordem: `carregador` (o menor), `placar-mesa`, `freedoom`. Para
   cada um: compilar, rodar os testes nativos, **gravar na placa e me pedir para conferir** tela,
   toque, som e cartão antes de passar para o próximo. Um commit por projeto.
4. Atualizar a CI e o README (seção "Software que funcionou" e a do Steam Deck) com as versões
   novas. Se as duas pastas do PlatformIO deixarem de ser necessárias, diga como juntar de volta.
5. **Fora do escopo** (só comente se valer a pena): LVGL 8 → 9 no fork (é uma migração grande
   da interface inteira) e o esp-web-tools do gravador.

## Restrições

- **Nunca** apague nem sobrescreva `JC3248W535/backup/`.
- A placa aparece como `/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_28:84:85:49:B7:1C-if00`.
  `/dev/ttyACM0` é o controle do Steam Deck: **sempre** passe a porta da placa.
- O `placar-mesa/platformio.ini` tem `build_dir` do Windows (`C:/Users/...`); no Linux use
  `PLATFORMIO_BUILD_DIR`. Mantenha isso funcionando nos dois sistemas.
- O fork compila o código de `placar-mesa/` e `freedoom/` (pastas irmãs). Depois de mexer neles,
  confirme que o fork **ainda compila**.
- Testes: os de `placar-mesa` rodam no PC (`pio test -e native`, dentro do distrobox `dev-gcc`).
  Todos têm que continuar passando.

## Regra para correções de bug

Toda correção de bug começa com um **teste que falha no código antigo**. Depois de corrigir,
**reverta a correção temporariamente** e rode o teste de novo para confirmar que ele falha sem
ela, porque vários "testes de regressão" passavam dos dois jeitos. Se o bug estiver em código
sem teste no PC (driver, hardware), diga isso claramente em vez de fingir cobertura.

## Como reportar

Em português, curto: o que foi atualizado, o que mudou no código e por quê, o que foi testado
na placa e o que não foi.
