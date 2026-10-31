# V6: vai e vem continuo de 180 graus, sem ACS

## Preparar os dois pontos uma vez por reinicializacao

O usuario confirmou que consegue marcar meia volta no eixo. Com o acoplamento
direto, marque dois pontos A e B separados por 180 graus REAIS, ambos dentro
do curso do potenciometro e afastados dos batentes. Nao use os batentes como
referencia e nao force o redutor/potenciometro para posicionar; desacople,
alinhe e recoloque se necessario.

1. TT sem alimentacao; RP2040 pelo USB; Serial em 115200.
2. Coloque o eixo no ponto A e envie **a**.
3. Coloque no ponto B, meia volta distante de A, e envie **b**.
4. Coloque o eixo no meio entre A e B.
5. Alimente a ponte e envie **i**.

O TT gira brevemente, detecta o sentido enquanto gira e passa automaticamente
ao vai e vem entre os pontos A e B. Continua ate x ou uma falha. Nao precisa
girar o B10K. A deteccao inicial tem teto de 1 segundo, encerrando a identificacao
ao observar 12 ADC de deslocamento; transfere imediatamente ao controle de
posicao. Nao existe espera adicional de calibracao.

O programa nao mede sozinho o angulo fisico. A e B devem ser medidos por voce:
registrar dois valores ADC arbitrarios nao garante meia volta. A escala exibida
e 0 graus em A e 180 graus em B. Aceita os dois sentidos do potenciometro.
Pontos ficam somente na RAM: registrar novamente apos reiniciar.

## Operacao

| Comando | Funcao |
|---|---|
| a / b | Para os motores e registra os respectivos pontos fisicos |
| i | TT faz vai e vem continuo A-B com deteccao automatica de sentido |
| t | TT faz vai e vem; servo normal acompanha o mesmo alvo relativo |
| s | Somente servo normal controlado pelo B10K |
| x | Para ambos, limpa falha e conserva os pontos a/b; nao reinicia movimento |
| ? | Ajuda |

O alvo avanca inicialmente a 20 graus/s: cerca de 9 segundos por meia volta,
com pausa de 300 ms em cada ponta. Esse e o ritmo do alvo, nao uma garantia
da velocidade fisica instantanea do motor. O TT recebe PWM 200/255 (78%)
somente quando precisa corrigir posicao; perto do alvo, desliga o acionamento.
Pode movimentar em pequenos passos. Usa tolerancia de 1,5 grau em cada ponta.
Nao houve ajuste fisico do controle; nao e um PID sintonizado.

Em t, o servo normal recebe 1000-2000 us de acordo com o alvo do TT; o angulo
real do servo normal depende do seu modelo e pode diferir de 180 graus.
Em s, recebe o comando do B10K como antes.

## Ligacoes mantidas

| Componente | Ligacao |
|---|---|
| B10K | Extremos 3V3/GND; cursor GP26 |
| Potenciometro de posicao, acoplamento direto ao TT | Extremos 3V3/GND; cursor GP27 |
| MX1508 IN1 / IN2 | GP6 / GP7 |
| TT | Saidas do canal A correspondente a IN1/IN2 |
| Servo normal | Sinal GP18; fonte adequada ao modelo e GND comum |
| GP28 / ACS712 | Sem uso; nao existe leitura ou corte por corrente |

Fonte +5V diretamente ao positivo da MX1508 se o ACS tiver sido retirado do
caminho. Fonte negativa, ponte negativa e GND RP2040 unidos. RP2040 pelo USB.
Nao alimentar motores pelo 3V3. Numeros GP sao GPIOs, nao pinos fisicos.

Mantidos: corte por ultrapassar A-B em mais de 3 graus, retorno proximo das
alimentacoes, falta de progresso e movimento contrario durante acompanhamento.
O teste inicial exige o eixo entre 75 e 105 graus do arco marcado. Fechar o
Serial interrompe quando a desconexao USB e detectada. Falhas desligam ambos
e pedem x; nao reiniciam sozinhas.

O desligamento usa IN1=IN2=0, sem freio ativo. Pode haver movimento por
inercia. Software nao garante impedir danos com sensor defeituoso ou
acoplamento escorregando. Comece sem carga e com acesso para desligar a fonte.

Abra teste_tt_v6_vaivem_180.ino mantendo Curso180.h junto. Usa biblioteca
Servo incluida no Arduino-Pico de Earle Philhower. Selecione sua variante
exata de placa RP2040. Compilacao e teste fisico sao etapas separadas.
