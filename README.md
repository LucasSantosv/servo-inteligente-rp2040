# Servo inteligente experimental com RP2040

Protótipo de baixo custo para estudar controle de posição de um motor DC com
redutor, retorno por potenciômetro e ponte H. O objetivo futuro é desenvolver
atuadores para um robô pato. **Este repositório registra uma experiência de
bancada em andamento, com versões que também falharam.**

![Primeira montagem experimental com TT, potenciômetro e RP2040](media/2026-10-31/foto_05.jpeg)

## Estado atual

- O usuário relatou que o TT girou após a retirada do ACS712 do controle.
- A pinagem e a ponte do canal A foram conferidas pelo usuário; alimentação
  de 5 V foi informada na entrada da ponte.
- Houve falhas de leitura do ACS, falta de movimento e bloqueio por calibração.
- Ainda **não foi validado** um servo inteligente estável, um vai e vem de
  180° físicos, controle de torque, ajuste PID ou operação contínua sob carga.
- A V7 foi compilada e corrige a exigência de marcar 180° à mão para começar.
  Ela usa um arco pequeno automático em ADC; ainda não foi testada fisicamente.

## Começar pela versão atual

Abra [teste_tt_v7_auto_sem_marcacao.ino](firmware/atual/teste_tt_v7_auto_sem_marcacao/teste_tt_v7_auto_sem_marcacao.ino)
no Arduino IDE, mantendo `Curso180.h` na mesma pasta. Usa a biblioteca Servo
incluída no pacote Arduino-Pico de Earle Philhower. A compilação foi verificada
para Raspberry Pi Pico, pacote 6.0.0; selecione a variante exata da sua placa.

1. Com o TT sem alimentação, confira que o potenciômetro está longe dos batentes.
2. Alimente a montagem e abra o Monitor Serial em **115200**.
3. Envie **`i`**. A V7 detecta o sentido e inicia um vai e vem de curso pequeno.
4. Envie **`x`** para parar. Não é preciso registrar `a` e `b`.

| Comando | Função |
|---|---|
| `i` | TT em vai e vem automático |
| `t` | TT e servo comum seguindo o mesmo alvo relativo |
| `s` | Servo comum no GP18 controlado pelo B10K |
| `x` | Para ambos e reconhece uma falha |
| `r` | Apaga referências manuais opcionais e volta ao automático |

O modo automático trabalha em **±160 unidades ADC** da posição inicial. O
Serial mostra percentual do curso, porque o ângulo físico total do
potenciômetro ainda não foi identificado. Detectar o sentido não calibra
automaticamente 180°. Os pontos manuais continuam opcionais para um arco
físico medido no futuro.

## Ligações desta experiência

| Função | Ligação |
|---|---|
| B10K de comando | Cursor GP26, extremos 3V3/GND |
| Potenciômetro acoplado ao TT | Cursor GP27, extremos 3V3/GND |
| MX1508 IN1 | GP6 |
| MX1508 IN2 | GP7 |
| Motor TT | Saídas do canal A correspondente a IN1/IN2 |
| Servo comum | Sinal GP18, fonte adequada ao modelo e GND comum |
| ACS712 / GP28 | Ignorados na versão atual |

Os números GP são GPIOs, não posições físicas dos conectores. A corrente do
motor retorna diretamente à fonte; o GND da fonte, da ponte e do RP2040 é
comum. O RP2040 pode ficar pelo USB. Se o ACS for retirado do caminho da
alimentação, o positivo da fonte vai diretamente ao positivo da MX1508.
Não alimente motores pelo pino 3V3.

O servo comum conserva sua eletrônica original. O protótipo com TT usa um
segundo potenciômetro acoplado diretamente ao eixo de saída, com a montagem
mostrada nas fotos. A faixa e a corrente nominal exatas desse TT ainda não
foram confirmadas. Fotos registram o experimento e não certificam a fiação.

## O que há neste repositório

- [Firmware atual e instruções](firmware/atual/teste_tt_v7_auto_sem_marcacao/COMECE_AQUI.md)
- [Histórico dos códigos experimentais](firmware/historico/)
- [Diário dos testes de 31/10/2026](docs/testes-2026-10-31.md)
- [Fotos e capturas do Serial](media/2026-10-31/README.md)
- [Arquitetura, componentes e próximos passos](docs/arquitetura-e-proximos-passos.md)
- [Como verificar a compilação e os testes](docs/validacao.md)

## Limites atuais

A potência de partida que chegou a produzir movimento foi mantida como base
experimental: PWM 200/255 (78%). O retorno de posição interrompe o acionamento
quando o alvo é alcançado. Os valores ainda precisam de ajuste físico; o alvo
gradual não garante velocidade instantânea constante. O movimento pode ocorrer
em pequenos passos. O controle atual não é um PID sintonizado.

Não há monitoramento de corrente na versão atual. O desligamento da ponte
usa IN1=IN2=0, sem freio ativo; pode haver movimento por inércia. Software
não garante impedir dano com retorno defeituoso ou acoplamento solto. Esta
montagem é para teste sem carga, com acesso para desligar a alimentação.

Encoder magnético de 360°, DRV8876, comunicação entre atuadores e uma PCB
para 20 unidades são possibilidades futuras, ainda não decisões fechadas.
Nenhuma PCB, capacidade de carga ou custo final foi validado. Licença de
distribuição ainda não definida.
