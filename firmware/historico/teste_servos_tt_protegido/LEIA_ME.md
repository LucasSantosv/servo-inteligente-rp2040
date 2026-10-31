# TT com retorno por potenciometro: teste protegido

Substitui o teste anterior para esta montagem. O programa anterior permanece
separado. O usuario confirmou acoplamento direto entre eixo de saida do TT e
potenciometro de posicao. Conserve teste_servos_tt_protegido.ino e LimitesTT.h
juntos na mesma pasta. Usa Arduino-Pico de Earle Philhower e a biblioteca Servo
incluida nesse pacote.

## O que foi corrigido

- Verificacao inicial de sentido limitada a 40 ms e PWM 32/255 (12,5%),
  permitida somente no centro do curso. Para antes se detectar deslocamento
  suficiente. Nao aumenta a potencia automaticamente se o teste falhar.
- Aceita os dois sentidos do motor e os dois sentidos eletricos do potenciometro.
- Durante o controle, corta ambos os testes se a posicao andar no sentido
  contrario ao comando por pelo menos 8 unidades ADC desde a ultima referencia.
- Corte se o retorno saltar mais de 8 graus nominais entre amostras.
- Corte se nao houver progresso suficiente durante 150 ms de comando nao nulo,
  inclusive com PWM pequeno. Corte do ACS inicial foi mantido.
- Nao liga o TT sem extremos a/b, confirmacao c, zero z e sentido v.
- Ao alterar extremos, registrar c, reconhecer uma falha com x ou apos uma
  falha, e preciso verificar novamente o sentido antes de ativar o TT.

## Limite de 180 graus: depende de calibracao FISICA

O programa nao conhece o curso angular total do potenciometro. Nao presume
que ele tenha 270 graus. Dois valores ADC arbitrarios NAO provam 180 graus.

Com a alimentacao do TT desligada, marque dois pontos seguros A e B no eixo,
separados por no maximo 180 graus (meia volta), dentro do curso do potenciometro
e afastados dos batentes fisicos. Pode comecar com um arco menor para o primeiro
teste. Use marcas e transferidor; nao use os batentes como referencia.
Se necessario desacople para posicionar e recoloque o acoplamento alinhado;
nao force o redutor ou o potenciometro.

O acoplamento e direto: o arco do TT coincide com o do potenciometro.
Registre A com a e B com b. So depois envie c para declarar que mediu esse
arco e que ele tem NO MAXIMO 180 graus. O comando c nao mede o angulo.
Se modificar o acoplamento ou fiacao, refaca a calibracao e o teste de sentido.

O software representa esse arco como 0-180 graus nominais:

| Regiao | Comportamento |
|---|---|
| A = 0, B = 180 | Referencias fisicas que VOCE mediu; nao sao alvos de movimento |
| 15 a 165 | Faixa comandada pelo B10K |
| <=10 ou >=170 | Desliga a ponte e trava falha |
| 75 a 105 | Faixa que permite iniciar a verificacao de sentido v |

Se A-B medir exatamente 180 graus, os valores nominais correspondem ao arco
fisico. Se A-B medir menos, o movimento fisico sera proporcionalmente menor.
O controle tem rampa de alvo de ate 30 graus nominais/s; isso nao e uma
garantia da velocidade real do motor. Tem zona morta de 2 graus nominais.
PID com KI=KD=0 inicialmente: somente controle proporcional, sem ajuste fisico.

As margens, baixa potencia e cortes reduzem o risco, mas o motor pode continuar
girando por inercia depois do corte. Software nao garante impedir danos com
sensor defeituoso, acoplamento escorregando ou cabo solto. Comece sem carga,
com fonte limitada e acesso imediato para cortar a alimentacao do TT.
Esta versao usa IN1=IN2=0 para desligar a ponte; nao implementa freio ativo.

## Ligacoes mantidas

| Funcao | Ligacao |
|---|---|
| B10K comando | Extremos em 3V3/GND, cursor no GP26 |
| Potenciometro de posicao | Extremos em 3V3/GND, cursor no GP27 |
| MX1508 | IN1 GP6, IN2 GP7; saidas desse canal no motor TT |
| Servo comum | Sinal GP18, fonte adequada ao modelo e GND comum |
| ACS712 | VCC 5V, GND comum, OUT pelo divisor 10k/10k ao GP28 |

Fonte (+) -> ACS IP+; ACS IP- -> MX1508 (+). Negativo da fonte -> MX1508 (-)
e GND RP2040. O servo comum recebe positivo em ramo separado, antes do ACS.
RP2040 pode usar USB. Os numeros sao GPIOs, nao pinos fisicos.
Use tensao adequada a variante do TT; nao presuma a tensao do MG995 anterior.
Nunca alimente os motores pelo 3V3 da placa.

O ACS712 ainda nao tem versao identificada: mostra ADC e variacao de OUT em
volts, sem amperes. O limite inicial e 0,05 V antes do divisor. Pode cortar na
partida; nao aumente esse limite nem o PWM sem identificar o sensor e revisar
o conjunto. Nao ha limitacao de corrente rapida por hardware neste programa.

## Sequencia do primeiro teste

1. Alimente somente o RP2040 pelo USB. Mantenha o TT sem alimentacao.
2. Monitor Serial 115200. Envie ? para ajuda. Confira POS ao reposicionar o
   potenciometro: deve mudar suavemente. Sem retorno confiavel nao acione TT.
3. Marque e registre A (a) e B (b), dentro de um arco fisico <=180 graus.
   O programa rejeita extremos proximos das alimentacoes e curso <300 ADC.
4. Envie c somente depois de medir esse arco. Posicione o eixo no centro.
5. Alimente a ponte na tensao adequada ao TT, mantendo motor parado. Envie z.
6. Envie v uma vez. O pulso tem ate 40 ms. Se o resultado for inconclusivo,
   confira a montagem; nao repita indefinidamente nem aumente potencia sozinho.
7. Depois de "Sentido verificado", B10K aproximadamente no meio: envie i.
   Comece com pequenas alteracoes no B10K.
8. x para interromper. Para reativar TT apos x, reposicione no centro e repita v.

## Comandos e testes independentes

| Comando | Funcao |
|---|---|
| s | Apenas servo comum pelo B10K, GP18; nao requer calibracao do TT |
| i | Apenas TT com retorno de posicao |
| t | Servo comum e TT seguem o mesmo B10K |
| x | Para ambos, limpa falha e invalida a verificacao de sentido do TT |
| a / b | Para e registra os dois extremos fisicos |
| c | Para e confirma que o arco A-B foi medido e e <=180 graus |
| z | Para e mede o zero do ACS712 |
| v | Verifica sentido a partir do centro, com pulso limitado |
| ? | Ajuda |

Calibracao somente em RAM: repetir apos reset. Fechar o Serial interrompe os
testes assim que a desconexao for detectada. N e I indicam modos ativos;
F=1 e falha travada. B10K/POS/ACS sao ADC 0-4095, dOUT e variacao em volts
antes do divisor; PWM e a saida assinada.

O servo comum conserva 1000-2000 us e sua eletronica original. Sem retorno
de posicao proprio, nao podemos garantir que esse intervalo corresponda a
180 graus fisicos. x desliga seus pulsos, nao a sua alimentacao eletrica.

Consulte VALIDACAO.md. Nao houve gravacao ou teste fisico nesta montagem.
