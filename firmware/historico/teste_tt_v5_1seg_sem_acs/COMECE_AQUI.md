# V5 - partida de ate um segundo, sem ACS

1. TT desligado: deixe o potenciometro de posicao perto do meio do curso.
2. Alimente a ponte, abra o Serial em 115200 e envie i.
3. Confira "V5 SEM ACS: partida 78%, ate 1 segundo, limitada por posicao".
4. Aguarde "Sentido detectado" e gire o B10K. x para interromper.

Comandos mantidos: s = servo normal; i = TT; t = os dois; x = parar.

## Alteracao solicitada

Partida em 78% (PWM 200/255), por ATE 1000 ms. A deteccao agora permite
80 ADC de deslocamento antes de encerrar, em lugar dos 12 ADC anteriores.
O programa desliga antes de um segundo se atingir esse deslocamento ou
alguma protecao. Assim, um pulso mais curto pode ser o resultado esperado.
Se chegar a um segundo com deslocamento de 12-79 ADC, aceita o sentido;
com deslocamento menor, para com falha. Nao repete automaticamente.

Depois da deteccao, aguarda 100 ms desligado para confirmar o retorno e
segue o B10K. O acompanhamento conserva 25% de PWM, com pulsos de 15 ms
e pausas de 35 ms. Esse acionamento ainda depende de teste fisico para
confirmar se vence o atrito da montagem.

Mantidos: janela pequena de +/-120 ADC ao redor da posicao inicial, margem
de 40 ADC para corte, posicao inicial proxima do centro (1000-3095 ADC),
cortes por salto do retorno, sentido contrario durante o acompanhamento,
falta de progresso e desconexao Serial. Nao existe escala angular calibrada:
nao se trata de um comando garantido de 180 graus.

## Ligacoes mantidas

| Funcao | Ligacao |
|---|---|
| B10K comando | Extremos 3V3/GND; cursor GP26 |
| Potenciometro acoplado ao TT | Extremos 3V3/GND; cursor GP27 |
| MX1508 IN1 / IN2 | GP6 / GP7 |
| Motor TT | Duas saidas correspondentes do canal A |
| Servo normal | Sinal GP18; fonte adequada e GND comum |
| GP28 | Sem uso; ACS ignorado |

Se retirar o ACS do caminho da alimentacao, fonte (+5V) direto ao positivo
da MX1508. Fonte negativa, ponte negativa e GND do RP2040 unidos. RP2040
pelo USB. Nao alimentar os motores pelo 3V3.

Nao ha leitura ou corte por corrente. O motor pode continuar por inercia
apos desligar o acionamento; nao ha freio ativo. Protecoes de software nao
garantem impedir dano com retorno defeituoso ou acoplamento solto. Testar
sem carga e manter acesso para desligar a fonte do TT.

Abra teste_tt_v5_1seg_sem_acs.ino mantendo ControleTT.h junto. Usa Servo
incluida no Arduino-Pico de Earle Philhower. Selecione a variante exata da
sua placa. Nenhum movimento ao ligar/resetar. Nenhuma gravacao ou teste
fisico foi realizado aqui.
