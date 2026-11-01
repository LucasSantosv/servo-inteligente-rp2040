# V9: PID completo, mais forca de acompanhamento e movimento suave

Base: V8, que o usuario relatou funcionar na montagem, com forca um pouco
baixa e necessidade de maior suavidade. A fonte, a ponte e a pinagem foram
mantidas. A V8 permanece em pasta separada para comparacao.

## Como testar

1. Grave teste_tt_v9_pid_suave.ino, mantendo Servo216.h junto.
2. TT desligado: eixo longe das pontas, preferencialmente perto do meio.
3. Alimente a ponte, abra Serial em 115200 e envie **i**.
4. Confira a mensagem **V9 PID**. Ele detecta sentido e faz vai e vem.
5. **x** interrompe os dois testes. Nao precisa marcar a/b.

O intervalo continua 18 a 198 graus nominais: 180 graus calculados para o
potenciometro de 216 graus informado. Isso pressupoe que 0-3V3 no cursor
correspondam ao curso eletrico util de 216 graus e que o acoplamento seja
direto. Nao e uma medicao angular externa.

## Mudancas

**PID completo.** P corrige erro de posicao, I compensa erro persistente
sob carga e D usa velocidade filtrada para amortecer o movimento. A integral
tem limite e anti-windup, para reduzir acumulo quando a saida satura. Os
ganhos sao iniciais; ainda precisam de teste e possivel ajuste na bancada.

**Mais acionamento disponivel.** O limite normal passou de 128/255 (50%)
para 200/255 (78%). Perto das pontas, de 96/255 (38%) para 160/255 (63%).
O PID usa essa potencia conforme a necessidade. Isso aumenta a margem de
comando, sem comprovar torque, corrente ou capacidade mecanica do conjunto.

**Aceleracao e desaceleracao graduais.** O alvo usa uma curva quintica:
velocidade e aceleracao comecam e terminam em zero. A duracao respeita os
limites configurados de velocidade, aceleracao e variacao da aceleracao
(jerk). Cada percurso completo de 180 graus tem alvo de cerca de 11,25 s;
o tempo real pode aumentar enquanto o eixo se aproxima da ponta.

**Compensacao de atrito continua.** Durante o acompanhamento, os chutes de
partida da V8 foram substituidos por compensacao suave, integral e variacao
gradual do PWM. O pequeno pulso inicial para identificar o sentido continua,
seguido de 200 ms para acomodacao antes da trajetoria.

**Retencao nas pontas.** Durante a pausa de 300 ms, o PID continua mantendo
a posicao; o TT nao e simplesmente solto como na V8. Em caso de x, falha ou
desconexao Serial, o acionamento e desligado imediatamente, sem esperar rampa.

## Parametros iniciais em Servo216.h

| Parametro | Valor |
|---|---:|
| KP / KI / KD | 9 / 6 / 1,8 |
| Velocidade maxima do alvo | 30 graus/s |
| Aceleracao maxima do alvo | 60 graus/s2 |
| Jerk maximo do alvo | 240 graus/s3 |
| Limite PWM normal / perto da ponta | 200 / 160 de 255 |
| Subida / descida maxima do comando PWM | 900 / 3000 unidades por segundo |
| Tolerancia de chegada | 1,2 grau nominal |

Esses sao limites do comando calculado. Eles nao garantem a velocidade,
aceleracao ou suavidade fisica do TT, que dependem de atrito, folga, carga,
fonte, fixacao e leitura do potenciometro.

## Comandos e ligacoes

| Comando | Funcao |
|---|---|
| i | TT em vai e vem com PID e aceleracao suave |
| t | Mesmo teste, com servo comum acompanhando o alvo relativo |
| s | Somente servo comum no GP18, pelo B10K |
| x / r | Para ambos e reconhece falha; nao reinicia sozinho |
| ? | Ajuda |

GP6 -> IN1, GP7 -> IN2 da MX1508, motor nas saidas do canal A correspondente.
B10K: cursor GP26, extremos 3V3/GND. Potenciometro do eixo: cursor GP27,
extremos 3V3/GND. Servo comum: sinal GP18 e fonte adequada ao seu modelo.
GP28 e ACS712 continuam ignorados. GND da fonte, ponte e RP2040 comum;
o retorno do motor vai direto a fonte. Nao alimentar motores pelo 3V3.

## Protecoes e observacao

Mantidos corte fora de 9-207 graus nominais, bloqueio de comando para fora
de 18-198 graus, intervalo desligado ao inverter a ponte, parada por x,
falha e desconexao Serial. A deteccao de falta de progresso foi ajustada:
nao confunde imediatamente PWM baixo de aproximacao com travamento; corta
apos mais de 1 s sem progresso com PWM aplicado >=140, ou apos 8 s sem
progresso e erro maior que 2,5 graus.

Nao ha medicao nem limitacao de corrente neste firmware. O PID e a integral
podem manter o motor energizado para sustentar carga. Teste inicialmente sem
carga e interrompa se houver aquecimento anormal ou oscilacao. O desligamento
nao garante parada mecanica imediata por causa da inercia.

No Serial: ANG e a posicao estimada, ALVO e o comando, VEL e a velocidade
estimada, VT e a velocidade do alvo, ACC e sua aceleracao, PWM e o comando
aplicado. Para comparar, observe especialmente ANG acompanhando ALVO e os
valores de VT/ACC mudando gradualmente. FALHA=1 indica parada travada.

Usa Servo incluida no Arduino-Pico de Earle Philhower. Selecione a variante
exata da placa. Compilacao e simulacoes aprovadas nao substituem teste fisico.
