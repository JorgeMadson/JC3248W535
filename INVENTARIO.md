# Inventário de hardware e descobertas

O que eu tenho para usar com a JC3248W535, o estado de cada peça e o que já foi
descoberto. Atualizado em 12/11/2026.

Legenda: ✅ testado e funcionando · ⚠️ com ressalva · ❓ ainda não testado · ❌ não usar

## Placa principal: JC3248W535 (Guition, ESP32-S3)

Detalhes de chip, tela e pinos no [README](README.md).

| Item | Estado | Notas |
|---|---|---|
| Tela + toque | ✅ | Usados no `placar-mesa/` |
| Alto-falante (P6) | ✅ | I2S BCLK=42, LRCLK=2, DATA=41. Pinos confirmados com um firmware de teste de tons; o sistema de fábrica tocou música do SD |
| Bateria (P5) | ⚠️ | Leitura no GPIO5 deu 2457 mV no pino (≈ 3,27 V pelo divisor 33k/100k). **Falta comparar com o multímetro** |
| Cartão SD | ⚠️ | O sistema de fábrica tocou música do SD, então o slot funciona; os pinos (CS 10, MOSI 11, SCK 12, MISO 13) vêm do esquema e não foram testados no nosso código |
| Rádios | ✅ | `esptool chip_id` responde `Features: WiFi, BLE`: **sem Bluetooth clássico**, então não vira caixa Bluetooth |
| Conectores P2 / P3 / P4 | ❓ | Pinagem do esquema publicado ([done.land](https://done.land/components/microcontroller/families/esp/esp32/developmentboards/esp32-s3/guitionjc3248w535/)), não testada. Ver abaixo |
| Cabos dos conectores | ❌ não tenho | Todos os conectores são JST 1,25 mm (P4 é 1,0 mm) |

### Conectores de expansão (do esquema, não testados)

| Conector | Tipo | Pinos |
|---|---|---|
| P1 | JST 1,25 mm, 4 vias | 1 VIN (5 V, só com USB ligado) · 2 TX (GPIO43) · 3 RX (GPIO44) · 4 GND |
| P2 | JST 1,25 mm, 8 vias | 1 GPIO5 (**bateria**, não usar) · 2 GPIO6 · 3 GPIO7 · 4 GPIO15 · 5 GPIO16 · 6 GPIO46 (**strapping**, evitar) · 7 GPIO9 · 8 GPIO14. **Sem 3,3 V nem GND** |
| P3 | JST 1,25 mm, 4 vias | 1 GND · 2 3,3 V · 3 GPIO17 · 4 GPIO18 |
| P4 | JST 1,0 mm (QWIIC), 4 vias | Igual ao P3 (mesmos fios, em paralelo) |

## Peças avulsas

### Da JBL Go 3 (molhada na piscina, desmontada)

| Peça | Marcação | Estado | Notas |
|---|---|---|---|
| Alto-falante | HSS03221, 4 Ω, conector micro JST 2 vias | ✅ | Encaixa direto no P6. Funcionou na placa |
| Bateria | GSP383562, LiPo 3,7 V 750 mAh (3,8 × 35 × 62 mm) | ⚠️ | Suspeita: na JBL o som melhorava no carregador (bateria sem força?). Medir tensão; descartar se estufar ou esquentar |
| Placa da JBL | — | ❌ | **Dois capacitores esquentam** (curto por água ou oxidação). O limpa-contato fez funcionar por algumas horas uma vez. Não ligar com bateria |

Diagnóstico da JBL (12/11/2026, testes por Bluetooth com o Steam Deck):
- Pelo USB ela não aparece em nada: o USB-C da Go 3 só carrega.
- Saíam só os agudos. Graves e médios viravam um ruído ritmado, tipo "morse", que **tinha
  sempre o mesmo volume**, por mais que o volume subisse. Isso é típico de amplificador
  entrando em proteção e religando, não de alto-falante estragado.
- Ela não informa a porcentagem da bateria pelo Bluetooth.

### Bateria Rontek 903048

| Marcação | Estado |
|---|---|
| 903048, LiPo 3,7 V 1300 mAh (9,0 × 30 × 48 mm), etiqueta `35489-RTLIPO903048-1300`, "polymer battery with circuit board and battery cell" | ❓ não testada |

- "With circuit board" = tem **placa de proteção** (corta sobrecarga, descarga total e curto). Ótimo.
- Tem **3 fios** e o P5 da placa só tem 2. O terceiro costuma ser um **termistor (NTC)** para
  medir temperatura. Não confirmado. Para conferir com o multímetro: entre o terceiro fio e o `−`
  deve dar uns 10 kΩ em temperatura ambiente (se for NTC de 10k).
- Na placa: ligar só `+` e `−` no P5 (`+` no pino mais perto do USB-C) e **isolar o terceiro fio**.
  Conferir a polaridade com o multímetro antes de encaixar.
- Tem quase o dobro da carga da bateria da JBL, por isso é a melhor opção para a placa.

### NRF24L01 com antena preta (PA+LNA)

| Estado | ❓ não testado, sem cabos |
|---|---|

- Rádio de 2,4 GHz da Nordic com protocolo próprio. **Não é Bluetooth nem Wi-Fi**: serve para
  ligar placas entre si (precisa de outro NRF24 do outro lado).
- Versão PA+LNA: mais alcance, mas puxa mais de 100 mA nos picos. Precisa de **capacitor de
  47–100 µF** colado no módulo, ou de um **adaptador NRF24L01 5V** (com regulador próprio).
- **Só 3,3 V. 5 V queima.**
- Ligação planejada (não testada): SCK GPIO6 · MOSI GPIO7 · MISO GPIO15 · CSN GPIO16 ·
  CE GPIO9 · IRQ GPIO14 (opcional), tudo no P2. Alimentação pelo P3 (3,3 V e GND) ou, com o
  adaptador, pelo VIN/GND do P1. Usar o barramento SPI3 (HSPI): a tela ocupa o SPI2.
- Primeiro teste planejado: analisador de espectro de 2,4 GHz na tela, que funciona com um
  módulo só.

### 2× DHT22 (temperatura e umidade)

| Estado | ❓ não testados |
|---|---|

- Sensor digital de um fio, funciona com 3,3 V. Lê no máximo uma vez a cada 2 s.
- Precisa de resistor de pull-up (~10 kΩ) no fio de dados. Muitos módulos já vêm com ele: confira.
- Ligação planejada: os dois no **P3** (3,3 V, GND, GPIO17 para um e GPIO18 para o outro), o
  que deixa o P2 livre para o NRF24.
- Uso planejado: o projeto "clima dentro e fora de casa" do README. Um sensor na placa, outro
  lá fora, ligado por fio comprido ou mandando os dados por NRF24.

## Para comprar

- [ ] Cabo JST 1,25 mm **8 vias** (P2), de preferência com ponta Dupont fêmea
- [ ] Cabo JST 1,25 mm **4 vias** (P3 e P1), idem. Comprar alguns
- [ ] Adaptador NRF24L01 5V (regulador + capacitores), ou capacitor de 47–100 µF
- [ ] Jumpers Dupont fêmea-fêmea
- [ ] Para o NRF24 conversar com alguém: um segundo NRF24 + um microcontrolador (Arduino Nano ou ESP32)

## Ambiente de desenvolvimento

- Steam Deck numa dock. PlatformIO 6.2.0 instalado (ver README, seção "No Steam Deck").
- Uma porta do hub não achou a placa, outra porta da dock funcionou.
- `/dev/ttyACM0` é o controle do Deck: gravar sempre pelo caminho fixo da placa.
