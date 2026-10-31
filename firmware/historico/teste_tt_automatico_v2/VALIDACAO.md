# V2 - pulso de partida

Compilacao aprovada para Raspberry Pi Pico (rp2040:rp2040:rpipico), pacote
Arduino-Pico 6.0.0. Codigo: 61.900 bytes; RAM global: 9.148 bytes.

A unica alteracao de acionamento e o PWM na deteccao inicial de sentido:
64/255 -> 112/255 (25% -> 44%). Pulso continua 15 ms a cada 50 ms,
por no maximo 200 ms. Acompanhamento continua PWM 64/255, com mesmo curso,
mesmas pausas e mesmas protecoes. Nao ha aumento automatico de potencia.

Testes em tempo de compilacao de ControleTT.h aprovados: intensidade e
duracao dos pulsos, partida no centro, faixa pequena e corte por retorno
contrario nos dois sentidos. Revisao da limitacao de PWM por estado:
112 somente em Detectando; 64 nos demais estados.

Motivo: usuario confirmou 5V na ponte, canal A e ligacao ja usada no teste
da antena. Na versao anterior, nao houve movimento fisico nem mudanca de POS.
Pulso fraco e uma hipotese a testar, nao causa comprovada.

Sem gravacao ou teste fisico da V2. Variante fisica RP2040 e versao do ACS712
ainda nao confirmadas. O curso e limitado em ADC, sem escala angular calibrada.
Versao anterior preservada na pasta teste_tt_automatico.
