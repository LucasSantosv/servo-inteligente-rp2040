# V9 - verificacao

## Base e estado fisico

Usuario relatou que a V8 funcionou, mas com pouca forca e movimento que
deveria ser mais suave. Esse relato nao equivale a medir torque ou angulo
externamente. A V8 foi preservada. Nao houve gravacao ou teste fisico da V9.

## Compilacao final

Aprovada para rp2040:rp2040:rpipico, Arduino-Pico 6.0.0.
Programa: 64.612 bytes. Variaveis globais: 9.496 bytes.
Biblioteca Servo incluida no pacote Arduino-Pico.

## Testes usando as mesmas classes do firmware

- Oito trajetorias de comprimentos/sentidos diferentes: monotonia do alvo,
  limites de velocidade/aceleracao/jerk, ponto final, velocidade e aceleracao
  finais nulas. Aprovadas.
- PID com KI ativo, inicio por rampa, retencao de compensacao de carga e
  limites da integral/saida. Aprovados.
- Monitor de progresso: corte com potencia alta e eixo parado; ausencia de
  corte imediato com PWM baixo; teto de falta de progresso prolongada.
  Aprovado. O teste identificou a necessidade de distinguir aproximacao
  suave de travamento, e esse criterio foi corrigido.
- 81 modelos simplificados de planta, 75 s cada: velocidades maximas
  120/300/600 graus/s, constantes de inercia 0,03/0,08/0,15 s, limiares de
  atrito 0/50/90 PWM e cargas constantes equivalentes a -25/0/+25 PWM.
  Incluem quantizacao ADC, intervalo desligado ao inverter, PID mantido na
  pausa e o mesmo monitor de progresso. Todos alcancaram os dois lados
  repetidamente, sem corte indevido nem saida da margem nominal de 9-207
  graus no modelo. Zero falhas nos 81 casos.

Os modelos nao foram identificados por medicoes do TT real. Eles verificam
o algoritmo antes da bancada, mas nao comprovam suavidade, torque, corrente,
precisao angular ou seguranca do conjunto fisico. Os ganhos sao iniciais.

Sem ACS e sem limite de corrente por firmware, conforme pedido anterior.
A escala de 216 graus continua baseada no curso informado pelo usuario.
