# V10 - verificacao

## Compilacao

Aprovada para rp2040:rp2040:rpipico, Arduino-Pico 6.0.0.
Programa: 64.340 bytes. Variaveis globais: 9.408 bytes.
Ponte configurada diretamente pelo SDK PWM do RP2040, GP6/GP7 no mesmo slice.
Servo comum permanece na biblioteca Servo por PIO.

## Evidencia que motivou a correcao

Usuario confirmou curso livre na parada de 193,4 graus. Captura posterior
mostra outra parada em 189,0 graus, PWM=160 e alvo aproximando-se de 198.
V9 impunha esse teto perto das pontas e seu monitor acusava falta de progresso
a partir de PWM>=140. A saturacao nao prova sozinha qual e a carga fisica,
mas o proprio firmware limitava o esforco que poderia tentar.

## Testes

- Mesmo perfil cinematico: monotonia e limites de velocidade/aceleracao/jerk.
- 108 modelos simplificados de 75 s: velocidades 120/300/600 graus/s,
  inercia 0,03/0,08/0,15 s, atrito 0/50/90/160 PWM e cargas -25/0/+25 PWM.
  Todos completaram os dois lados repetidamente sem sair da margem no modelo.
- Regressoes de inicio: 189 e 193,4 graus aceitos; retomada com sentido conhecido
  respeita os limites e parte para dentro.
- 511 comandos de ponte, -255 a +255: simetria de direcao, niveis 0-256,
  zero ativo como freio, parada como roda livre, um canal alto no PWM-B.
- Progresso: PWM160 nao causa o corte rapido antigo, mas parada com PWM200
  ainda e detectada em pouco mais de um segundo.
- Modelo adicional de atrito de partida alto, iniciado em 189 graus e alvo198:
  V9 parou em 189,00; V10 chegou a 196,80, dentro da tolerancia de 1,2 grau,
  sem corte. Trata-se de modelo de regressao, nao medicao do motor do usuario.

Todos esses testes passaram. O modelo mecanico simplificado nao representa
a recirculacao eletrica do PWM-B nem foi identificado na montagem real.
A logica eletrica foi conferida pelo datasheet e pelos testes de niveis;
efeito fisico, corrente e aquecimento ainda precisam de ensaio de bancada.

Referencia primaria: MX1508 Rev1.3, pagina10, modos A/B e faixa10-50kHz:
https://file.thegioiic.com/upload/documents/1734591971_MX1508.pdf

Sem gravacao ou teste fisico da V10. Sem ACS por solicitacao do usuario.
Escala de 216 graus ainda nominal, baseada no curso informado. V8/V9 preservadas.
