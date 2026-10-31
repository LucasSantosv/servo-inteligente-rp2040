# Teste conjunto: servo comum e servo inteligente MG995

Um unico programa, com B10K compartilhado. Nenhum movimento ao ligar.
O servo comum conserva sua eletronica original. No MG995 convertido, a placa
original deve estar isolada dos dois fios do motor e dos tres terminais do
potenciometro; engrenagens e acoplamento ao eixo permanecem montados.

## Ligacoes

Os numeros abaixo sao GPIOs (GP), nao numeros fisicos dos conectores.
A variante exata da placa RP2040 ainda nao foi confirmada.

| Ligacao | Destino |
|---|---|
| B10K: extremos | 3V3 e GND |
| B10K: cursor | GP26 |
| Potenciometro interno MG995: extremos | 3V3 e GND |
| Potenciometro interno MG995: cursor identificado pelo multimetro | GP27 |
| MX1508: IN1 / IN2 | GP6 / GP7 |
| MX1508: duas saidas correspondentes a IN1/IN2 | Dois fios do motor MG995 |
| Servo comum: sinal | GP18 |
| Servo comum: positivo | Fonte regulada adequada ao seu modelo |
| Servo comum: negativo | Negativo da fonte |
| ACS712: VCC / GND | 5V regulados / GND |
| ACS712: OUT | Resistor 10k -> GP28; outro resistor 10k de GP28 ao GND |
| Fonte do motor: positivo | ACS712 IP+ |
| ACS712 IP- | MX1508 positivo |
| MX1508 negativo e GND RP2040 | Negativo da fonte |
| RP2040 | USB do computador |

O divisor 10k/10k do ACS712 e obrigatorio neste esquema. Nao alimentar o
ACS712 por 3V3 e nao ligar sua saida diretamente ao GP28.
O caminho de corrente do ACS alimenta somente a ponte do MG995: o servo
comum deve receber seu positivo em um ramo separado, antes do sensor.
Se ambos passarem pelo ACS, o programa confundira a corrente dos dois.

Usar uma fonte regulada adequada ao MG995, inicialmente 5V, com limitacao de
corrente para o teste sem carga. A alimentacao do servo comum depende do seu
modelo. Nao alimentar motores pelo 3V3 da placa; unir os GNDs e levar os retornos
dos motores diretamente a fonte. O ACS712 nao limita corrente por hardware.
A capacidade real da MX1508 precisa ser validada para o motor desse MG995.

## Servo comum: teste imediato

1. Desligue a alimentacao da MX1508 para este primeiro teste.
2. Abra o Monitor Serial em 115200 baud e envie `?` para ver a ajuda.
3. Coloque o B10K aproximadamente no meio e envie `s`.
4. Gire o B10K: somente o servo comum segue o comando. Envie `x` para parar.

O intervalo do servo comum e 1000-2000 us. Nao representa necessariamente
0-180 graus; o angulo real depende do servo. Um servo de rotacao continua
respondera com velocidade/sentido. Ao ativar, recebe imediatamente a posicao
do B10K. `x` remove os pulsos; nao corta sua alimentacao eletrica nem garante
que todos os modelos deixem de segurar a ultima posicao.

## Preparar o inteligente

Com a ponte desligada e sem carga mecanica, identifique dois pontos que
delimitem um curso seguro, antes dos batentes. Nao force o trem de engrenagens.
Se o MG995 nao permitir reposicionar suavemente, interrompa e ajuste a montagem
para calibrar sem forcar. O potenciometro deve acompanhar o eixo de saida.

1. Posicione o eixo no primeiro ponto seguro. Envie `a`.
2. Posicione no outro ponto seguro. Envie `b`.
3. Reposicione no meio desse curso. Os extremos podem ter ADC crescente ou
   decrescente; o programa aceita ambos. Exige diferenca de pelo menos 300 ADC.
4. Ligue a ponte, com o motor parado. Envie `z`: mede o zero do ACS.
5. Envie `v`: aplica um pulso de ate 150 ms, limitado a 25% de PWM, e observa
   o sentido do retorno. Termina assim que detecta movimento suficiente.
6. Se aparecer "Sentido verificado", coloque o B10K no meio e envie `i`.
7. Depois de conferir funcionamento e limites, envie `t` para os dois servos
   seguirem o mesmo B10K. Eles compartilham o comando relativo, nao uma
   garantia de angulos ou velocidades fisicas identicas.

Se o sentido for inconclusivo ou houver corte, confira motor, ponte, retorno,
fonte e ACS antes de alterar parametros. Nao insista em um motor travado.

## Comandos

| Comando | Acao |
|---|---|
| s | Apenas servo comum |
| i | Apenas servo inteligente; exige calibracao, zero e sentido verificado |
| t | Os dois juntos; exige a mesma preparacao do inteligente |
| x | Para os dois e limpa falha; nao reinicia movimento |
| a / b | Para os dois e registra o respectivo limite de posicao |
| z | Para os dois e mede o zero do ACS |
| v | Para os dois e verifica o sentido com um pulso limitado |
| ? | Ajuda |

Aceita maiusculas e qualquer terminacao de linha. Calibracoes ficam somente
na RAM: repetir depois de reiniciar. Fechar/desconectar o Serial tambem
interrompe o teste assim que a desconexao e detectada pelo USB.

## Controle e protecoes iniciais

O MG995 usa controlador PID com KI=KD=0 inicialmente: portanto, nesta versao
o controle ativo e proporcional. O ajuste fino depende da montagem real.
A leitura ocorre a cada 5 ms; o alvo avanca gradualmente e fica entre 3% e 97%
do curso calibrado. Ha zona morta de 1% e PWM maximo 64/255 (cerca de 25%).
O programa corta a ponte por retorno fora da faixa, falta de progresso
(500 ms sob comando relevante) ou tres leituras seguidas do ACS acima do
limite. Esses cortes sao complementares; nao garantem detectar fio solto,
curto ou corrente de pico. Testar sem carga com fonte limitada.

A versao do ACS712 ainda e desconhecida. O programa mostra `ACS` em ADC e
`dOUT` em volts de variacao absoluta da saida original, antes do divisor.
Nao mostra amperes. O limite inicial `LIMITE_DELTA_OUT_V = 0.05` equivale
nominalmente a aproximadamente 0.27 A (5A), 0.50 A (20A) ou 0.76 A (30A).
Pode cortar ja na partida do MG995; esse valor precisa de revisao apos
identificar o codigo do sensor e medir o conjunto. Ruido/offset tambem afetam
a leitura. Com PWM, mede corrente de alimentacao da ponte, nao toda a corrente
instantanea do enrolamento. A versao costuma estar marcada 05B, 20A ou 30A
no chip; uma foto legivel permite confirmar.

`B10K` e `POS` sao leituras 0-4095; `PWM` e a saida assinada; `N` e `I`
indicam os testes ativos; `F=1` indica falha travada. Corrija a causa e envie x.

## Arduino IDE

Usa Servo incluida no pacote Arduino-Pico de Earle Philhower, versao instalada
6.0.0. Selecione a variante exata da sua placa RP2040 no Arduino IDE e abra
teste_servos_juntos.ino. Nao gravar uma imagem Pico numa variante diferente
sem confirmar a placa. O arquivo anterior teste_servo_b10k e um rascunho
separado; para os dois servos use somente esta pasta.

Nao houve gravacao na placa nem validacao fisica. Consulte VALIDACAO.md para
o resultado da verificacao de compilacao.
