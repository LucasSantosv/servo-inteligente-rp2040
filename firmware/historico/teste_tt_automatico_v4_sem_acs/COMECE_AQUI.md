# V4 - teste automatico sem ACS712

1. Com o TT desligado, deixe o potenciometro de posicao perto do meio do curso.
2. Alimente a ponte e abra o Monitor Serial em 115200 baud.
3. Envie i. Confira a mensagem "V4 SEM ACS".
4. Aguarde "Sentido detectado" e gire o B10K lentamente. x para parar tudo.

| Comando | Funcao |
|---|---|
| i | TT detecta sentido e segue B10K |
| s | Apenas servo normal |
| t | Os dois juntos, com deteccao automatica do TT |
| x | Para e reconhece uma falha; nao reinicia movimento |
| ? | Ajuda |

## Ligacoes

| Componente | Ligacao |
|---|---|
| B10K comando | Extremos 3V3/GND; cursor GP26 |
| Potenciometro de posicao acoplado ao TT | Extremos 3V3/GND; cursor GP27 |
| MX1508 IN1 / IN2 | GP6 / GP7 |
| TT | Duas saidas do canal A correspondente a IN1/IN2 |
| Servo normal | Sinal GP18; alimentacao adequada ao modelo; GND comum |
| GP28 | Sem uso; pode desconectar OUT e divisor do ACS712 |

Se retirar o ACS712 do caminho de alimentacao, reconecte o positivo da fonte
de 5V diretamente ao positivo da MX1508. Negativo da fonte, negativo da ponte
e GND RP2040 unidos. RP2040 pode ficar pelo USB. Nao alimentar motor pelo 3V3.
O sensor tambem pode permanecer no caminho de alimentacao: o programa nao
le, nao calibra e nao usa sua saida para interromper o teste.

## Comportamento mantido

Partida 78% (200/255), por ate 50 ms, com pausas de 35 ms. A tentativa de
detectar sentido termina em ate 200 ms ou antes se detectar deslocamento.
Depois, acompanhamento com PWM 25%, pulsos de 15 ms e pausas de 35 ms.
Nao aumenta potencia automaticamente. Se nao mover, para com mensagem.

Mantidos: curso inicial pequeno (+/-120 ADC), margem de 40 ADC para corte,
verificacao de movimento contrario, salto de retorno, falta de progresso e
parada por x/desconexao Serial. Partida exige posicao proxima do centro
eletrico (1000-3095 ADC). Sem calibracao angular: esse curso nao e declarado
como 180 graus. Acoplamento direto foi informado pelo usuario.

Esta versao nao monitora corrente nem corta o motor por sobrecorrente.
O corte da ponte usa IN1=IN2=0, sem freio ativo; pode haver movimento por
inercia. As protecoes de posicao nao garantem impedir dano com retorno
defeituoso ou acoplamento solto. Testar sem carga e com acesso para desligar.

Servo comum: 1000-2000 us, angulo fisico depende do modelo; x remove o sinal,
nao a alimentacao. Nenhum motor se move automaticamente ao ligar/resetar.

Abra teste_tt_automatico_v4_sem_acs.ino, mantendo ControleTT.h junto na mesma
pasta. Usa a biblioteca Servo incluida no Arduino-Pico de Earle Philhower.
Selecione a variante exata da sua placa. Nao houve gravacao ou teste fisico.
