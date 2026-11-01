# V13 — PID, B10K, Plotter e ACS712

Abra teste_tt_v13_pid_acs712.ino mantendo os quatro arquivos .h na mesma pasta. Placa de compilação: Raspberry Pi Pico (RP2040, Arduino-Pico). Serial: 115200 baud. A V12 foi preservada em sua própria pasta.

## Ligações

| Função | Ligação |
|---|---|
| B10K comando | Cursor GP26; extremos 3V3/GND |
| Potenciômetro de posição | Cursor GP27; extremos 3V3/GND |
| MX1508, motor A | IN1 GP6, IN2 GP7, motor na saída A |
| ACS712 VCC | 5 V |
| ACS712 GND | GND comum ao RP2040 e à ponte |
| ACS712 OUT | Divisor 10k/10k abaixo, ponto central GP28 |
| Fonte do motor positivo | ACS IP+ |
| ACS IP− | Positivo de alimentação da MX1508 |
| Fonte negativo | GND da ponte e GND comum |
| Servo normal opcional | Sinal GP18, alimentação externa adequada e GND comum |

```text
ACS OUT ── 10k ──┬── GP28
                 │
                10k
                 │
                GND

Fonte + ── IP+ [ACS] IP− ── positivo MX1508
```

Não ligar OUT diretamente ao GP28. Não aplicar 5 V nas entradas analógicas. A alimentação do RP2040 por USB e a do servo normal ficam fora do caminho medido pelo ACS. A medição é da alimentação da ponte, não de toda a corrente que recircula pelo enrolamento durante PWM/frenagem.

## Primeiro teste

1. Monte com a alimentação desligada. Depois alimente o ACS e a ponte e deixe o motor parado, eixo próximo do meio e B10K no meio. Não deve haver corrente pelo ACS durante o zero. Como ele está antes da ponte, a pequena corrente de repouso da ponte pode entrar no zero e ser subtraída das leituras seguintes.
2. Abra o Serial em 115200 e envie **z**. Ele para ambos os motores, espera 500 ms e coleta 128 médias em aproximadamente 640 ms, sem travar a execução. Aguarde cerca de **1,2 segundo** sem enviar outro comando de movimento.
3. No Plotter, **ACS_OK** deve passar para 1. No Monitor, envie **l** para ler a confirmação ou a rejeição. Se rejeitar, confira alimentação, GND e divisor e repita z.
4. Se conhecer a versão, envie **1** para 5 A/05B, **2** para 20 A ou **3** para 30 A. Use a inscrição do sensor; esses números são seleções, não limites de corrente. Sem identificação, mantenha **0**, a opção inicial.
5. Envie **g** para usar o gráfico e **p** para seguir o B10K suavemente. **f** mantém o modo rápido com até 100% de PWM. **x** para.

Apenas a medição foi acrescentada. O ACS não impede começar o movimento se não estiver calibrado, e não há corte por corrente nesta versão. A versão desconhecida não recebe uma escala inventada em amperes. As proteções de posição, ausência de progresso e atraso do controle foram mantidas.

## Curvas e valores

- **Alvo/Posicao:** referência e medição em graus nominais.
- **P/I/D/FF:** contribuições do controle antes da saturação/rampa; FF compensa atrito e velocidade.
- **PWM:** comando assinado efetivamente enviado à ponte.
- **Falha:** falha do controle de movimento, 0/1. O motivo fica disponível em l.
- **ACS_mV:** diferença filtrada em relação ao zero, em milivolts na saída OUT do ACS, antes do divisor. Tem sinal e funciona com a versão desconhecida após z.
- **Corrente_mA:** corrente estimada em miliamperes, com sinal; válida somente com **Amp_OK=1**. Por exemplo, 1000 mA = 1 A.
- **ACS_OK:** zero aceito e leitura ADC fora das extremidades. Não comprova que todos os fios estão conectados ou que o módulo é preciso.
- **Amp_OK:** ACS_OK e escala 1/2/3 selecionada. Não comprova que o usuário escolheu o modelo correto.

Se ACS_OK=0, ACS_mV fica em zero como valor indisponível. Se Amp_OK=0, Corrente_mA fica em zero como valor indisponível: não interpretar como ausência de corrente. Sem calibração use l para observar ACS_ADC bruto.

Desmarque curvas para comparar apenas PID/posição ou corrente. O Plotter compartilha a escala vertical, e uma curva em mA pode comprimir as curvas em graus. São enviados até 20 quadros/s, descartando quadros quando o USB não tem espaço. A corrente é a média filtrada das leituras disponíveis, não RMS nem captura dos picos rápidos de PWM.

## Comandos

| Comando | Ação |
|---|---|
| z | Para ambos e mede o zero do ACS |
| 0 | ACS desconhecido: somente variação em mV |
| 1 / 2 / 3 | ACS 5 A / 20 A / 30 A |
| p | TT segue B10K suavemente com PID |
| f | TT segue B10K rápido com PID, até 255/255 PWM |
| i | Vai e vem automático do TT |
| t | Vai e vem TT e servo normal |
| s | Servo normal pelo B10K |
| g | Saída para Plotter |
| l | Monitor com mensagens e leitura bruta do ACS |
| v | Monitor com velocidade e pico |
| x | Para e reconhece falha |
| r | Para e apaga sentido aprendido |
| ? | Ajuda no Monitor |

z não reconhece uma falha de movimento existente; nesse caso confira a causa e use x. Enviar p/f/i/t/s durante a medição de zero a cancela. A calibração e a escala ficam na RAM: após reiniciar, envie z e selecione a versão novamente. Não há reinício automático. Fechar o Serial pode parar o motor; selecione o modo novamente após abrir a outra janela.

## Controle preservado

A malha roda a cada aproximadamente 5 ms. Em p, o alvo mantém limites de 45°/s, 90°/s² e variação de aceleração de 360°/s³; PWM máximo 200/255. Em f, mantém PID e até 255/255, sem a mesma rampa suave. A detecção de sentido usa pulsos de 200/255. i/t mantêm o perfil automático da V12.

A escala de posição ainda assume ADC 0–4095 equivalente a 0–216°. O curso comandado é 18–198° (180° nominais), com margem externa de corte 9–207°. Isso não é uma calibração física comprovada. RPM e graus/s também dependem dessa escala.

## Escala de corrente

A conversão usa ADC nominal de 3,3 V, divisor 2:1 e sensibilidades típicas de 185, 100 e 66 mV/A para as versões 5, 20 e 30 A, respectivamente. Fonte: [datasheet Allegro ACS712](https://www.allegromicro.com/-/media/Files/Datasheets/ACS712-Datasheet.ashx).

Tolerâncias dos resistores, referência ADC, alimentação do ACS, ruído e temperatura afetam o resultado. O zero remove o deslocamento inicial; não calibra o ganho. Para conferir amperes reais, compare com um instrumento de referência. O zero aceita média no ADC entre 0,9 e 1,6 V e variação pico a pico de até 100 contagens nas 128 médias; esses critérios só verificam plausibilidade.
