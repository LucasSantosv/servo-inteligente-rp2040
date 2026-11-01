# V10: corrigir parada perto da ponta e melhorar o acionamento

## Testar

1. Grave teste_tt_v10_pwm_freio.ino com Servo216.h e PonteMX1508.h na mesma pasta.
2. Serial em 115200; envie **i**. Confira a identificacao **V10 PWM-B**.
3. Observe o eixo e os valores ANG/ALVO. **x** para e reconhece uma falha.
4. Depois de x, **i** retoma para dentro, usando o sentido ja aprendido no mesmo
   boot. **r** para e apaga esse aprendizado; use r se alterar fiacao/acoplamento.

Nao precisa marcar a/b. A pinagem continua GP6/GP7 na MX1508, GP27 no retorno,
GP26 no B10K, GP18 no servo normal, GP28/ACS sem uso. O teste i e automatico;
s usa B10K no servo normal; t faz os dois acompanharem o mesmo alvo relativo.

## O que as capturas mostraram

Na V9, o usuario registrou uma parada em ANG=193,4 e outra em ANG=189,0, ambas
tentando chegar a 198. O comando chegou ao teto de PWM=160 usado perto das
pontas e a velocidade caiu para perto de zero. O monitor entao travou o teste
com "Sem progresso de posicao". O usuario confirmou curso mecanico livre.
Isso aponta uma limitacao de comando/controle como hipotese; nao e uma
medicao de torque ou exclusao de toda causa mecanica/eletrica.

A tela tambem mostrou velocidade estimada acima da velocidade do alvo em
alguns momentos. O usuario percebeu movimento em pulos. A impressao da V9
ocorria apenas a cada 250 ms: valores entre linhas tambem sao naturalmente
discretos e nao demonstram sozinhos o movimento fisico.

## Correcao principal: modo PWM da ponte

O datasheet original da MX1508 distingue dois modos. No modo A, usado antes,
o acionamento alterna com alta impedancia/roda livre; o fabricante alerta
que PWM rapido pode dificultar a partida. No modo B, alterna com frenagem
e e indicado para controlar melhor a rotacao, recomendando 10-50 kHz.

A V10 usa modo B a aproximadamente 20 kHz. Uma entrada fica alta e a outra
recebe PWM complementar. Comando zero durante controle usa ambas altas
(frenagem); x/falha/estado parado usam ambas baixas (roda livre). GP6 e GP7
sao atualizados juntos pelo periférico PWM do RP2040, evitando intermediarios
de atualizacoes separadas. A duracao de 5 ms desligado ao inverter virou um
intervalo em frenagem enquanto o teste esta ativo.

Fonte: [MX1508, Rev1.3, pagina 10, modos PWM A/B](https://file.thegioiic.com/upload/documents/1734591971_MX1508.pdf).
O efeito eletrico real desse modo ainda precisa de teste nessa placa.

## Outros ajustes

- Teto perto das pontas: 160 -> 200/255 (o mesmo maximo ja permitido no centro).
  A potencia continua variavel pelo PID, e nao permanentemente no maximo.
- Integral pode compensar mais atrito/carga, mantendo limite e anti-windup.
- Corte rapido de falta de progresso passa a avaliar PWM >=190 por mais de
  um segundo, permitindo ao controlador tentar a potencia disponivel. O teto
  de 8 s sem progresso com erro grande permanece. Nenhum corte foi desativado.
- Inicio com sentido ainda desconhecido aceita 19-197 graus nominais, incluindo
  189 e 193,4 das capturas. Com sentido conhecido e posicao dentro da margem,
  a retomada inicia para o lado interno, sem novo pulso de identificacao.
- Perfil suavizado mais lento: velocidade maxima do alvo 20 graus/s,
  aceleracao maxima 40 graus/s2 e jerk maximo 160 graus/s3. Um percurso de
  alvo completo de 180 graus leva cerca de 16,9 s; o tempo real pode aumentar.
- Serial atualizado a cada 100 ms. A malha continua em passos nominais de 5 ms.

Mantido PID com KP=9, KI=6 e KD=1,8, velocidade filtrada, compensacao continua
de atrito e limitacao da variacao do PWM. Ainda sao ganhos iniciais, sem
sintonia identificada fisicamente. Folga, atrito e ruido podem limitar suavidade.

## Limites

Permanece o percurso nominal de 18-198 graus e corte externo de 9-207,
calculados do curso de 216 graus informado pelo usuario. A escala pressupoe
um potenciometro linear com 0-3V3 nesse curso eletrico util. ANG e uma
estimativa calculada do ADC; nao e uma medicao externa dos graus reais.
As capturas analisadas nao comprovam leitura saturada em 4095 no meio do curso.

Sem medicao ou limite de corrente por firmware. O novo modo muda a recirculacao
de corrente e a dissipacao da ponte; confira temperatura no ensaio sem carga
e interrompa se houver aquecimento anormal. Nao ha garantia de parada mecanica
instantanea ou protecao contra sensor defeituoso/acoplamento solto.

Fonte de 5 V foi informada nesta bancada. GND da fonte, ponte e RP2040 comum;
retorno do motor direto a fonte; RP2040 pode ficar pelo USB. Nao alimentar
motor pelo 3V3. Se o ACS saiu da alimentacao, fonte positiva direto a ponte.

Usa Arduino-Pico e sua biblioteca Servo. Esta implementacao de PWM foi feita
para GP6/GP7 (canais A/B do mesmo slice), nao para pinos arbitrarios.
Nenhum motor parte ao ligar/resetar. Compilacao/simulacao nao equivalem a
teste fisico. Consulte VALIDACAO.md.
