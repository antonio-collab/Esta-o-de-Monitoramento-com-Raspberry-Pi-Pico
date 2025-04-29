Claro! Aqui está a **documentação detalhada** do seu código explicando cada parte, seguida do **código limpo (sem comentários)**, conforme você pediu.

---

## 📝 **Documentação do Código - Estação de Monitoramento com Raspberry Pi Pico**

### 🎯 Objetivo Geral
Este programa é desenvolvido para rodar no **Raspberry Pi Pico (RP2040)**, utilizando dois núcleos de processamento. Ele monitora a posição de um joystick analógico e, dependendo da leitura do eixo X, aciona **LEDs coloridos** e um **buzzer PWM** para sinalização sonora.

---

### 🧩 Estrutura Geral do Programa

#### 1. **Definições de Pinos**
- `vRx` e `vRy`: conectados aos eixos X e Y do joystick (ADC0 e ADC1).
- `SW`: botão do joystick (entrada digital com pull-up).
- `led_r`, `led_g`, `led_b`: LEDs vermelho, verde e azul.
- `buzzer`: pino do buzzer (controlado por PWM).

#### 2. **Variáveis e Constantes**
- `flag_estado`: variável global `volatile` (compartilhada entre núcleos), usada para armazenar o valor do eixo X.
- `LIMIAR_BAIXO`, `MODERADO`, `ALTO`: determinam os limites de ativação de LEDs e buzzer.

---

### ⚙️ **Funções do Programa**

#### `pwm_init_buzzer(pin)`
Inicializa o pino do buzzer para funcionar com sinal PWM a 2kHz. A frequência é ajustada usando a função `pwm_config_set_clkdiv`.

#### `beep(pin, duration_ms)`
Ativa o buzzer com duty cycle de 50% por um tempo definido em milissegundos e depois o desliga.

#### `setup_joystick()`
Inicializa os canais ADC dos eixos do joystick e configura o botão (`SW`) como entrada com resistor pull-up.

#### `setup_saida()`
Configura os pinos dos LEDs como saída e inicializa o PWM para o buzzer.

#### `setup()`
Inicializa a comunicação serial, joystick e saídas. É chamada uma vez no início.

#### `joystick_read_axis(&x, &y)`
Lê os valores analógicos do eixo X e Y e armazena nas variáveis passadas por referência.

#### `core1_entry()`
Executado no segundo núcleo (core 1). Recebe os valores via FIFO (`multicore_fifo_pop_blocking`) e decide o comportamento:
- Valor alto → LED vermelho + buzzer (alarme)
- Valor moderado → LED azul
- Valor baixo → LED verde

#### `alarme_callback()`
Função chamada periodicamente por um timer. Envia o valor atual da variável global `flag_estado` para o core 1 via FIFO.

---

### 🧠 **Multicore**
O programa usa os **dois núcleos** do RP2040:
- **Core 0 (principal)**: coleta os dados do joystick e atualiza `flag_estado`.
- **Core 1**: processa os dados e ativa os LEDs/buzzer de acordo com os limiares.

Isso permite paralelismo eficiente: enquanto o core 0 lê e prepara dados, o core 1 executa as ações de saída.

---

### 🔁 **Main Loop**
No loop principal (`while (1)`), os eixos do joystick são lidos e `flag_estado` é atualizado. Um `repeating_timer` envia esses dados ao core 1 a cada 2 segundos.

---
