# Verificacao em 31/10/2026

- Compilacao APROVADA: rp2040:rp2040:rpipico, Arduino-Pico 6.0.0.
- Codigo: 61.884 bytes. Variaveis globais: 9.148 bytes.
- Testes em tempo de compilacao das mesmas regras ControleTT.h usadas no
  firmware: partida proxima do centro, janela de curso e margens, pulso/pausa,
  polaridade e deteccao de movimento contrario nos dois sentidos. APROVADOS.
- Revisao estatica: zero do ACS com ponte desligada, limite temporal da
  deteccao, confirmacao do retorno apos pausa, saida desativada no reset,
  parada por x/falha/desconexao Serial, e falha sem aumento de potencia.
- Nenhuma gravacao ou validacao fisica. Velocidade, corrente, inercia e
  integridade do acoplamento ainda dependem de teste na montagem real.
- Sem calibracao angular: usa curso pequeno em ADC; nao garante 180 graus.
- Versoes anteriores preservadas em suas respectivas pastas.
