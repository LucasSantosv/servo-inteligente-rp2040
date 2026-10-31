# V3 - partida em 78% com pulsos de ate 50 ms

Compilacao final aprovada para rp2040:rp2040:rpipico, Arduino-Pico 6.0.0.
Codigo: 61.900 bytes; RAM global: 9.148 bytes.

Partida: PWM 200/255 (78%), pulsos de ate 50 ms, pausas de 35 ms. A tentativa
inteira continua limitada a 200 ms e termina antes ao detectar deslocamento.
Acompanhamento pelo B10K permanece PWM 64/255, pulsos de 15 ms e pausas de
35 ms. Curso pequeno, verificacao de retorno, ACS e demais cortes mantidos.

Testes em tempo de compilacao aprovados para: limites da partida, curso,
pausas, bordas do pulso de 50 ms e deteccao de sentido contrario.

Usuario confirmou ausencia de movimento na V2, 5V na ponte e motor no canal A.
Partida fraca continua uma hipotese, nao uma causa comprovada. Nao houve
gravacao nem teste fisico da V3. Sensor ACS712 sem variante identificada;
curso angular nao calibrado. Versoes anteriores preservadas.
