# Prompt: controles do Freedoom, visual do Placar e tela inicial personalizável

> Cole tudo abaixo da linha num chat novo do Claude Code aberto em `~/dev` (Steam Deck), com os
> dois repositórios lado a lado: `JC3248W535/` e `JC3248W535EN-fork/`.

---

Placa **JC3248W535** (ESP32-S3, tela 3.5" 480×320 deitada, toque AXS15231B). O sistema que uso
é o launcher em português do fork (`JC3248W535EN-fork`, ESP-IDF 6.1 + LVGL 8.3), com dois apps
que vêm do outro repositório: **Placar** (`JC3248W535/placar-mesa`) e **Freedoom**
(`JC3248W535/freedoom`). Quero três coisas, nesta ordem. Faça uma de cada vez, grave na placa e
**me peça para testar** antes de passar para a próxima. Um commit por parte.

## Leia antes

- `JC3248W535/README.md` (seções Freedoom, Placar e "No Steam Deck") e `INVENTARIO.md`.
- `JC3248W535EN-fork/README.md` (início, em português).
- A memória do projeto no Claude Code: testes no distrobox `dev-gcc`, duas pastas do
  PlatformIO, e a regra dos testes (abaixo).

Como compilar e gravar (sempre passe a porta: `/dev/ttyACM0` é o controle do Deck):
```
cd ~/dev/JC3248W535EN-fork
PLATFORMIO_CORE_DIR=$HOME/.platformio-idf pio run -t upload --upload-port /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_28:84:85:49:B7:1C-if00
```
Serial: 115200 na mesma porta (o launcher escreve com `serial_log`; o Doom com prefixo `[doom]`).
O firmware próprio do Freedoom (`JC3248W535/freedoom`, Arduino, `pio run`) usa o mesmo código
comum: confirme que ele ainda compila quando mexer em `freedoom/src/comum/`.

## 1. Controles do Freedoom (o mais urgente)

Testei jogando e:
- **Não anda.** A alavanca de ANDAR (metade esquerda) não move o jogador.
- **ATIRAR gira a câmera.** Arrastar o dedo dentro do botão ATIRAR mexe a mira. Não gostei:
  quero que atirar só atire.
- O botão ATIRAR é grande demais; pode ser menor.

Onde está: `JC3248W535/freedoom/src/comum/entrada.c`. Ele transforma toques em teclas do Doom,
com as áreas desenhadas no comentário do topo, e as mesmas áreas aparecem como contornos em
`JC3248W535EN-fork/src/doom/doom_ui.cpp` (`montar_botoes`) e em
`JC3248W535/freedoom/src/placa.cpp` (`placa_desenhar_botoes`). Mude as três juntas.
O toque no launcher vem de `placa_ler_toques` (`doom_ui.cpp`), que lê até 2 dedos pelo
`bsp_touch_handle()`. Em 14/11/2026 o driver do toque foi alterado para ler 2 dedos
(`src/esp_lcd_axs15231b.c`) e **ninguém confirmou que o painel informa o segundo dedo**.

O que fazer:
1. **Descubra por que não anda antes de mudar qualquer coisa.** Ponha um log temporário dos
   toques (posição, quantos dedos, quais teclas saem) e me peça para tocar e arrastar na
   alavanca. Suspeitas a conferir: as coordenadas que chegam (giradas pelo
   `bsp_touch_process_points_cb`) não batem com as áreas; a leitura de 2 dedos devolve lixo no
   segundo; a zona morta/centro da alavanca; o Doom ignorar as setas por algum motivo.
2. Corrija com **teste primeiro**: a lógica de `entrada.c` pode ser separada em uma função pura
   (toques → teclas) e testada no PC (`pio test -e native` no `freedoom/`, dentro do
   `dev-gcc`), como já existem `test/test_zona` e `test/test_leitura`.
3. ATIRAR: só atira, sem girar. Botão menor. Proponha onde fica o giro (a alavanca já vira
   para os lados; talvez basta) e me mostre o layout novo antes (um desenho em texto como o do
   comentário de `entrada.c`).

## 2. Visual do Placar

O placar (`JC3248W535/placar-mesa`) já ganhou cartões, botões desenhados e barras, mas **ainda
está simplório e feio**. Quero que fique com cara de app bem feito.

- Antes de codar, **me mostre 2 ou 3 propostas** de visual (por exemplo uma página HTML com
  prévias 320×480 de cada tela: menu, Riftbound, Magic, resultado e diálogos) e espere eu
  escolher.
- Limites reais: o desenho é feito pela classe `Pintor`, que tem duas implementações com a
  **mesma interface**: `placar-mesa/src/hal/pintor.*` (Arduino_GFX) e
  `JC3248W535EN-fork/src/placar/pintor.*` (software, num `lv_canvas`). Sem anti-aliasing, fontes
  Logisoso da U8g2 (tamanhos fixos em `src/ui/tema.h`). Se uma proposta precisar de algo novo
  no `Pintor` (degradê, sombra, ícone), implemente nas duas.
- A tela é dividida ao meio: cada jogador lê a sua metade (a de cima é girada 180°).
- Mantenha os testes passando (`pio test -e native` no `placar-mesa`, 40 hoje).
- No fim, atualize as prévias SVG do card do Placar em `JC3248W535/site/index.html`.

## 3. Tela inicial do launcher personalizável

Arquivo principal: `JC3248W535EN-fork/src/home_ui.cpp` (grade 5×2 = 10 ícones,
`add_app_tile`). Hoje: Wi-Fi, Freedoom, Clima, Arquivos, Música, Rádio, Breakout, Placar,
Ajustes, Painel. Fora da tela mas com o código pronto: **Relógio** (`clock_ui_open`) e **Air**
(`air_ui_open`).

1. **Trocar o Placar com o Wi-Fi de lugar** (Placar vira o primeiro).
2. **Ícone melhor para o Freedoom.** Hoje é o símbolo de mira do LVGL. Desenhe um ícone
   original (por exemplo uma imagem pequena em C, `lv_img`), **sem** usar o logotipo nem a arte
   do Doom (marca registrada): algo que lembre um jogo de tiro.
3. **Editar a tela inicial nos Ajustes:** mudar a ordem dos ícones e mostrar/esconder cada app,
   para poder, por exemplo, esconder o Freedoom e trazer o Relógio de volta.
   - Lista de todos os apps num lugar só (nome, ícone, cor, função de abrir), em vez dos
     `add_app_tile` soltos.
   - Guardar no cartão, junto com os outros ajustes (`settings.ini`, ver `settings.cpp` /
     `settings_ui.cpp`), com um padrão quando não houver nada salvo.
   - **Ajustes nunca pode ser escondido** (senão não há como desfazer).
   - Se houver mais de 10 visíveis, a grade precisa rolar ou ter páginas; proponha.
   - A lógica de ordem/visibilidade (ler e gravar a lista, mover para cima/baixo, não deixar
     esconder os Ajustes) deve ser testável no PC; se o fork não tiver ambiente de teste nativo,
     crie um pequeno, ou deixe essa lógica num arquivo sem LVGL e teste com gcc no `dev-gcc`.
4. Atualize a prévia SVG da tela inicial no card "Sistema com apps" do site.

## Regra para correções de bug

Toda correção de bug começa com um **teste que falha no código antigo**. Depois de corrigir,
**reverta a correção temporariamente** e rode o teste de novo para confirmar que ele falha sem
ela, porque vários "testes de regressão" passavam dos dois jeitos. Cuidado com testes que passam
por acaso (em 14/11/2026 um teste da zona de memória passava sem a correção porque dois arrays
falsos ficavam colados na memória). Se o bug estiver em código que não dá para testar no PC,
diga isso claramente.

## Para publicar no site

Depois de cada parte aprovada: commit e push do fork; recompile **a partir do commit** (a versão
nos Ajustes vem do `git describe`); copie
`JC3248W535EN-fork/.pio/build/JC3248W535EN/firmware.factory.bin` para
`JC3248W535/site/firmwares/placar-os/placar-os.factory.bin`; commit e push do `JC3248W535` (a CI
publica o GitHub Pages e gera o SHA256SUMS). Confira o SHA-256 publicado em
`https://jorgemadson.github.io/JC3248W535/firmwares/placar-os/SHA256SUMS.txt`.

Responda sempre em português, curto, dizendo o que foi testado na placa e o que não foi.
