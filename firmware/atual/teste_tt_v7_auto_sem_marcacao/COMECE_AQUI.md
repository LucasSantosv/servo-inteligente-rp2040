# V7: iniciar sem marcar 180 graus

Grave o programa. Com o potenciometro acoplado ao TT longe dos batentes,
abra o Serial em 115200 e envie **i**. O programa usa a posicao atual como
centro, detecta o sentido e faz um vai e vem de curso pequeno automaticamente.
**x** para. **r** apaga referencias manuais opcionais e volta ao automatico.

Nao precisa enviar a/b. Um ponto manual B isolado, como na captura da V6,
nao impede iniciar. Se ambos os pontos manuais forem validos, r retorna ao
modo automatico. A faixa inicial de partida aceita POS entre 400 e 3695;
o valor POS=3550 mostrado na captura e aceito.

| Comando | Funcao |
|---|---|
| i | Vai e vem automatico do TT |
| t | TT e servo normal acompanhando o mesmo alvo relativo |
| s | Somente servo normal pelo B10K |
| x | Para ambos e reconhece falha |
| r | Limpa referencias manuais opcionais |

Automatico: +/-160 ADC ao redor da posicao inicial, com margem adicional
para corte; partida PWM 200/255 (78%), deteccao com teto de um segundo,
controle por retorno e sem ACS712. O Serial mostra CURSO% relativo, nao graus.
A escala fisica do potenciometro nao e conhecida: este modo nao promete 180
graus reais. O alvo leva cerca de 9 s por percurso completo, mas o movimento
fisico pode ocorrer em passos e ainda depende de ajuste de bancada.

Modo manual opcional: somente se futuramente houver dois pontos fisicos
medidos, registre a/b. O programa mantem essa possibilidade, mas nao a exige
para testar. Quando a escala e manual, o Serial mostra ANG. Os pontos ficam
na RAM e sao perdidos no reset.

Pinagem: GP6 IN1, GP7 IN2 da MX1508; motor no canal A correspondente.
GP26 B10K comando, GP27 potenciometro do eixo; extremos dos dois em 3V3/GND.
GP18 sinal do servo comum. GP28 sem uso. GNDs unidos; fonte adequada ao motor
na ponte (5V foi informado nesta bancada), RP2040 pelo USB. Se o ACS saiu do
caminho de alimentacao, positivo da fonte direto ao positivo da ponte.

Mantidos os cortes por posicao fora da faixa, falta de progresso e sentido
contrario durante acompanhamento, alem de x e desconexao Serial. Nao ha
corte por corrente ou freio ativo; inercia e falha do retorno podem causar
ultrapassagem. Teste sem carga e com acesso para desligar a alimentacao.

Abra teste_tt_v7_auto_sem_marcacao.ino mantendo Curso180.h junto. Usa Servo
incluida no Arduino-Pico de Earle Philhower. Compile para sua variante de
placa RP2040. Compilado para Pico; ainda sem teste fisico da V7.
