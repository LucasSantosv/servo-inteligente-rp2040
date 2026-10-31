# Teste simples do motor TT

## Como usar

1. Com o TT sem alimentacao, deixe o potenciometro de posicao perto do meio
   do curso. O eixo do TT e o potenciometro devem estar acoplados diretamente.
   Nao force o redutor; se precisar, desacople, alinhe e acople novamente.
2. Alimente a montagem com tensao adequada ao seu TT. Abra o Monitor Serial
   em 115200 baud.
3. Digite **i**. Aguarde "Sentido detectado".
4. Gire o B10K lentamente. O TT acompanha dentro de um curso pequeno.
5. Digite **x** para parar.

Nao precisa registrar a/b/c, zerar o sensor ou escolher a direcao manualmente.

| Comando | Teste |
|---|---|
| i | TT: detecta sentido automaticamente e segue B10K |
| s | Apenas servo normal no GP18 |
| t | Faz a mesma configuracao automatica do TT e testa os dois juntos |
| x | Interrompe ambos e reconhece uma falha; nao reinicia movimento |
| ? | Ajuda |

## O que ele faz sozinho

Ao receber i ou t, o TT permanece desligado enquanto o programa mede o zero
do ACS712. Em seguida, aplica pequenos pulsos sempre na mesma polaridade:
15 ms ligados a PWM 64/255 (25%), separados por pausas de 35 ms. Observa se
o retorno no GP27 aumenta ou diminui e configura a polaridade de controle.
Interrompe a deteccao assim que recebe movimento suficiente, ou apos 200 ms
sem sucesso. Aguarda mais 100 ms desligado para confirmar o deslocamento.
Nao procura extremos mecanicos nem aumenta potencia automaticamente.

Depois o B10K comanda uma faixa de apenas +/-120 ADC ao redor da posicao
inicial. Ha mais 40 ADC de margem para corte. O alvo avanca a ate 40 ADC/s.
Os pulsos reduzem o acionamento medio, mas nao garantem uma velocidade
fisica especifica; isso depende da tensao, do redutor e da carga do TT.
O eixo pode mover em pequenos passos. Esta versao usa controle de posicao
por pulsos, sem ajuste PID.

Se o potenciometro de posicao estiver longe do centro eletrico (fora de
1000-3095 ADC), o programa pede para centralizar e nao aciona o TT.
Essa verificacao nao substitui conferir o centro mecanico da montagem.

## Protecoes e limites deste teste

- Ponte desativada ao ligar/resetar. Movimento somente por i/t.
- Curso inicial pequeno e corte por sair da faixa ou leitura mudar bruscamente.
- Corte se o retorno indicar movimento contrario ao esperado.
- Corte se houver comando mas nao houver progresso suficiente em 250 ms.
- Monitoramento do ACS712, com o mesmo limite inicial conservador de 0,05 V
  de variacao em OUT, antes do divisor. A versao do ACS ainda e desconhecida;
  nao calcula corrente em amperes e pode cortar na partida.
- Falhas interrompem os dois testes e exigem x antes de uma nova tentativa.
- Fechar o Serial interrompe os testes quando a desconexao USB e detectada.

Este programa nao tem uma escala angular calibrada: **nao chama o curso de
180 graus**. Apenas detectar o sentido nao revela o curso total do
potenciometro. Comecamos com curso pequeno; para ampliar depois, precisaremos
identificar ou medir esse curso. Nenhuma busca automatica de batentes e feita.

Software nao garante impedir dano com retorno defeituoso, acoplamento solto
ou inercia. IN1=IN2=0 desativa o acionamento; nao aplica freio ativo. Teste
sem carga, com fonte limitada e acesso para desligar o TT.
Se parar por falta de movimento, nao aumente parametros antes de observar
o resultado e conferir a montagem.

## Ligacoes mantidas

| Componente | Ligacao |
|---|---|
| B10K comando | Extremos 3V3/GND; cursor GP26 |
| Potenciometro do eixo TT | Extremos 3V3/GND; cursor GP27 |
| MX1508 | IN1 GP6, IN2 GP7; saidas desse canal nos dois fios do TT |
| Servo comum | Sinal GP18; alimentacao adequada ao modelo; GND comum |
| ACS712 | VCC 5V; GND comum; OUT pelo divisor 10k/10k ao GP28 |

Fonte (+) -> ACS IP+; ACS IP- -> positivo da MX1508. Negativo da fonte,
negativo da ponte e GND do RP2040 unidos. Servo comum alimentado num ramo
separado, antes do ACS. RP2040 pelo USB. Nao alimentar motores pelo 3V3.
Os numeros GP sao GPIOs, nao posicoes fisicas dos conectores.

Servo comum usa 1000-2000 us. O angulo real depende do modelo; o comando s
aplica imediatamente a posicao do B10K. x remove pulsos, nao sua alimentacao.

Abra teste_tt_automatico.ino no Arduino IDE, mantendo ControleTT.h na mesma
pasta. Usa Servo do pacote Arduino-Pico de Earle Philhower. Selecione a
variante exata da placa RP2040. Nao houve gravacao nem teste fisico aqui.
