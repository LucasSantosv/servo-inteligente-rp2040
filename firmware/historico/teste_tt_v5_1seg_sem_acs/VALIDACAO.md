# V5 - verificacao

Compilacao aprovada: rp2040:rp2040:rpipico, Arduino-Pico 6.0.0.
Codigo: 61.500 bytes. RAM global: 9.128 bytes.

Testes em tempo de compilacao aprovados para o limite temporal de 1000 ms,
pulso de partida, deslocamento de deteccao dentro da janela nos dois sentidos,
limites de posicao e deteccao de sentido contrario durante acompanhamento.

Revisao estatica: estado de deteccao encerrado apos 1000 ms no maximo;
encerra antes ao atingir 80 ADC ou uma protecao; depois confirma retorno
com motor desligado. Aceita 12-79 ADC ao fim do tempo; menos gera falha.
Partida 78%, acompanhamento 25%, ACS ignorado e GP28 sem uso.

Sem gravacao ou teste fisico. A duracao fisica do acionamento pode ser menor
que um segundo por corte antecipado. Angulo e velocidade reais nao calibrados.
Versoes anteriores preservadas.
