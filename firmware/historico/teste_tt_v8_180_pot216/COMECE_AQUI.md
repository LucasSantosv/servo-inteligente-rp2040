# V8: vai e vem de 180 graus nominais com potenciometro informado de 216 graus

## Usar

1. Grave esta V8. Abra o Serial em 115200.
2. Com o TT sem alimentacao, deixe o eixo longe das pontas, preferencialmente
   perto do meio. Alimente a ponte na tensao usada no teste anterior.
3. Envie **i**. Aguarde a deteccao automatica do sentido.
4. O alvo passa a ir de **18 a 198 graus**, continuamente. **x** para tudo.

Nao precisa marcar a/b nem medir meia volta a mao. A faixa foi calculada como
(216 - 180)/2 = 18 graus de margem em cada ponta. No ADC de 12 bits os pontos
nominais sao aproximadamente 341 e 3754. A precisao fisica ainda depende do
sensor e do acoplamento; o programa nao mediu o angulo externamente.

**Premissa da escala:** os 216 graus informados pelo usuario correspondem ao
curso ELETRICO util de um potenciometro linear, com extremos em GND/3V3 e
cursor percorrendo 0-3V3 nesse arco. Se 216 for somente curso mecanico, outra
marcacao ou o sensor nao aproveitar toda a tensao nesse curso, o angulo real
diferira e sera necessario corrigir a escala. A inscricao por si so nao
garante seguranca contra inercia ou montagem incorreta.

## O que mudou em relacao a V7

- Saiu a janela pequena de +/-160 ADC. Entraram limites fixos calculados
  a partir dos 216 graus informados.
- Saiu o comando fixo de 78% sempre que havia erro. Agora a potencia varia
  com o erro de posicao e a diferenca entre velocidade atual e desejada
  (controle PD).
- Potencia normal limitada a 128/255 (50%), e a 96/255 (38%) perto das pontas.
- Para vencer atrito, pode aplicar um impulso de 200/255 (78%) por 12 ms
  nominais, observado em passos de 5 ms, com intervalo minimo de 200 ms
  entre inicios. O impulso termina antes se detectar velocidade ou
  deslocamento suficiente.
- Acionamento para fora e bloqueado ao alcancar 18/198 graus; correcoes
  para dentro continuam permitidas. Corte travado se sair de 9 a 207 graus.
- Ha intervalo de 5 ms desligado para inverter a ponte. Deteccao inicial
  usa pulsos de ate 20 ms e teto total de um segundo, seguida de 80 ms
  desligado para acomodar a inercia inicial.

O alvo avanca a 15 graus/s, aproximadamente 12 segundos para cada percurso
completo de 180 graus, com pausa de 300 ms em cada ponta. Esse e o ritmo do
alvo, nao uma garantia da velocidade instantanea do eixo. Tolerancia de
chegada: 1,2 grau nominal e velocidade estimada baixa antes de inverter.
Pode haver movimento em passos por causa do atrito e da folga do TT.
KP e KD sao pontos de partida; ainda nao houve sintonia fisica.

## Comandos

| Comando | Funcao |
|---|---|
| i | TT faz vai e vem entre 18 e 198 graus nominais |
| t | Mesmo teste do TT, com servo comum acompanhando o alvo relativo |
| s | Somente servo comum no GP18 controlado pelo B10K |
| x / r | Para ambos e reconhece uma falha; nao reinicia sozinho |
| ? | Ajuda |

Em t, o servo comum recebe 1000-2000 us. Isso nao comprova 180 graus fisicos
nesse segundo servo, cujo curso depende do modelo.

## Pinagem mantida

| Componente | Ligacao |
|---|---|
| B10K de comando | Extremos 3V3/GND; cursor GP26 |
| Potenciometro acoplado diretamente ao TT | Extremos 3V3/GND; cursor GP27 |
| MX1508 IN1 / IN2 | GP6 / GP7 |
| Motor TT | Duas saidas correspondentes do canal A |
| Servo comum | Sinal GP18, fonte adequada e GND comum |
| ACS712 / GP28 | Ignorados |

Fonte e ponte compartilham GND com o RP2040; o retorno do motor vai direto
a fonte. Se o ACS saiu do caminho da alimentacao, positivo da fonte direto
ao positivo da ponte. RP2040 pode ficar pelo USB. Nao alimentar motores
pelo pino 3V3. Os numeros GP sao GPIOs, nao pinos fisicos dos conectores.

## Serial e limites

`V8 POS` e a leitura ADC; `ANG` e a posicao estimada em 0-216 graus; `ALVO`
e a posicao pedida; `VEL` e a velocidade estimada em graus/s; `PWM` e o
comando assinado aplicado a ponte. `FALHA=1` para os dois testes.

O programa corta por ultrapassagem da margem nominal, ausencia de progresso
ou atraso grande da malha. Fechar o Serial interrompe quando o USB detectar
a desconexao. A confirmacao do sentido ocorre no inicio; durante o controle,
o sinal de PWM pode inverter para desacelerar, o que e intencional.

Sem monitoramento de corrente. Desligar o acionamento nao garante parada
mecanica imediata. Testar sem carga e manter acesso para cortar a alimentacao.
As margens nao substituem conferir o curso real e a fixacao do potenciometro.

Abra teste_tt_v8_180_pot216.ino e mantenha Servo216.h junto. Usa Servo incluida
no Arduino-Pico de Earle Philhower. Selecione sua variante exata RP2040.
Consulte VALIDACAO.md: compilacao/simulacao nao equivalem a teste fisico.
