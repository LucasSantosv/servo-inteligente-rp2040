# Verificacao em 31/10/2026

- Compilacao final APROVADA para rp2040:rp2040:rpipico, Arduino-Pico 6.0.0.
- Codigo: 64.468 bytes. Variaveis globais: 9.252 bytes.
- Testes de LimitesTT.h APROVADOS por static_assert no compilador C++:
  extremos crescentes/invertidos, calibracoes invalidas, conversao relativa,
  margens e ultrapassagem, pulso somente no centro, as quatro combinacoes
  de sentido motor/retorno, e corte por movimento contrario nos dois sentidos.
  Esses testes avaliam as mesmas funcoes usadas pelo firmware.
- Revisao estatica dos estados: reset nao move; a/b/c invalidam sentido;
  x/falha invalidam sentido; i/t exigem arco confirmado, zero e sentido;
  pulso v e limitado; protecoes interrompem os dois testes.
- Firmware anterior preservado em teste_servos_juntos.
- Nao houve gravacao, medicao angular, medicao de corrente, teste de inercia,
  verificacao fisica do acoplamento ou ajuste do controlador.
- Variante fisica da placa RP2040 e variante do ACS712 ainda nao confirmadas.

O teto de 180 graus depende de registrar dois pontos fisicos com arco medido
de no maximo 180 graus. O software nao consegue verificar essa medicao.
Nao se trata de uma garantia fisica contra dano por inercia ou falha do sensor.
