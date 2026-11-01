# V15 — Monitor lento e colunas fixas

Abra **teste_tt_v15_monitor_lento.ino**, com os cinco arquivos .h juntos, e grave na placa. Esta é uma variante da V15: mantém a mesma identificação e funciona com o Gravador V3 existente.

- Inicia no **Monitor Serial**, em 115200, com uma linha a cada **1 segundo** (antes: 100 ms).
- **l** seleciona esse Monitor lento, se estiver no Plotter.
- **f** inicia o modo rápido pelo B10K; **p** o suave; **x** para.
- Valores possuem largura fixa e uma casa decimal. Os espaços ficam depois de cada valor, mantendo os rótulos nas mesmas colunas e preservando a leitura pelo Gravador V3.
- PID, compensações, limites, sensores, posições e pinagem não foram alterados.
- O controle continua em aproximadamente 5 ms. A coleta c e o Plotter g/h continuam em até 20 quadros/s; somente o Monitor em texto foi desacelerado.
- Mensagens de comando e falha aparecem imediatamente. A limitação de um segundo se aplica às linhas periódicas, não aos avisos.

O Monitor da IDE é um histórico de linhas, não um painel que redesenha os números no mesmo lugar da tela. As colunas ficam fixas, mas as linhas continuam rolando. Se a janela estiver estreita, amplie-a ou desative a quebra de linha nas opções disponíveis na sua IDE. Não foram usados códigos de terminal para tentar apagar a tela.

O texto compacto do Monitor não inclui os ADCs brutos; os valores completos continuam disponíveis no modo c e nos arquivos do Gravador V3. Para uma tela com valores fixos sem rolagem, use a janela do gravador.

As instruções de controle e ligações abaixo são as da V15, com os mesmos ajustes do motor.

# V15 — ajuste do modo rápido e Plotter em duas telas

## O que mudou

No modo **f**, o ganho derivativo passa de **1,8 para 1,0**. O D continua sendo calculado a partir da velocidade filtrada e usado no comando real da ponte; não houve corte ou alteração apenas visual da curva. P, I, filtro da velocidade, compensação de atrito, rampa de PWM e limite de 255/255 foram mantidos.

No modo **p** e no vai e vem automático, o ganho D continua **1,8**. A pinagem e os sensores da V14 foram mantidos. A V14 original foi preservada em sua própria pasta.

## Primeiro teste

1. Extraia a pasta e abra **teste_tt_v15_monitor_lento.ino**, mantendo os cinco arquivos .h juntos. Compile/grave como Raspberry Pi Pico, Arduino-Pico/RP2040.
2. Para comparar o ajuste, comece com a fonte do motor em **5 V**, nas mesmas condições de carga anteriores. O ajuste não certifica o motor para 7,5 V.
3. Abra o Serial em **115200**, eixo e B10K aproximadamente no meio. Confira os sensores em h/l. Se desejar corrente em mA, faça z e selecione 1/2/3 conforme o ACS; sem identificação mantenha 0.
4. Envie **g** e depois **f**. Faça movimentos pequenos e pare o B10K: compare se a posição estabiliza. A meta é reduzir a oscilação da posição e as inversões de PWM, não apenas baixar o D no gráfico.
5. **x** para o motor. Se persistir tremor, pare o teste e grave um trecho pelo Gravador V3 para analisar posição, alvo, velocidade e PWM em conjunto.

Foi validado em software, não na sua montagem. O ajuste pode exigir novos valores conforme tensão, carga, folgas, ruído e motor. Não aumentar tensão para tentar corrigir oscilação.

## Plotter da IDE — oito curvas por tela

A IDE 2 tem uma limitação de oito variáveis documentada em https://github.com/arduino/arduino-ide/issues/651. A V14 transmitia 14 curvas; as que excediam oito podiam não aparecer.

- **g — PID:** Alvo, Posicao, P, I, D, FF, PWM, Falha. Selecione essa tela com g.
- **h — sensores:** PWM, Corrente_mA, ACS_mV, ACS_OK, Amp_OK, Temp_C, NTC_OK, Falha.
- **l — Monitor:** valores em texto, incluindo PID, corrente e temperatura, em colunas fixas.
- **c — coleta completa:** todos os campos em um quadro compacto; usado automaticamente pelo Gravador V3, sem limitação de oito sinais no arquivo.

Se a legenda não mudar ao alternar g/h, feche e reabra o Plotter. A perda de conexão pode parar o motor; envie f/p de novo quando o Serial estiver aberto.

D pode continuar ultrapassando o valor de PWM: as parcelas são exibidas antes de saturação e rampa, e o PWM final continua limitado. A amplitude de D, sozinha, não comprova estabilidade nem instabilidade.

## Ligações preservadas

| Função | Ligação |
|---|---|
| MUX VCC / GND / EN | 3V3 / GND / GND |
| MUX S0 / S1 / S2 / S3 | GP2 / GP3 / GP4 / GP5 |
| MUX SIG / COM | GP26 |
| MUX C0 | Cursor B10K; extremos 3V3/GND |
| MUX C1 | Junção do divisor NTC abaixo |
| MUX C2–C15 | Reservados; ainda não lidos |
| Potenciômetro de posição | Cursor GP27; extremos 3V3/GND |
| ACS712 OUT | Divisor 10k/10k ao GP28; VCC 5 V; GND comum |
| MX1508 IN1 / IN2 | GP6 / GP7; TT na saída A |
| Servo normal | Sinal GP18, fonte externa adequada e GND comum |

```text
3V3 ── resistor fixo 4,7k ──┬── C1
                           │
                        NTC 100k
                           │
                          GND
```

Sensor configurado como 100k a 25 °C e Beta 3950, conforme o código da extrusora; ainda precisa de confirmação física. NTC lido em 10 Hz e filtrado. **Temp_C só vale com NTC_OK=1. Corrente_mA só vale com Amp_OK=1.** Zeros com indicadores inválidos não representam medições reais de zero.

O ACS mede a alimentação da ponte. OUT não deve ir diretamente ao GP28; o caminho de potência é fonte + -> IP+ -> IP− -> positivo da ponte. Não aplicar 5 V às entradas analógicas ou canais do multiplexador.

## Gravação

Use **GravadorDeTestesV3**. Reconhece V13, V14 e V15 e mantém os nomes automáticos. Na V15 seleciona c e grava o quadro D15 completo: PID, alvo, posição, velocidade, pico, PWM, corrente e temperatura com estados dos sensores e tempo do RP2040.

O gravador V2 não identifica V15. Feche o Monitor/Plotter da IDE antes de conectar pelo gravador. Arquivos em Documentos > ServoInteligente > Testes. O nome do teste registra o modo solicitado, não comprova sucesso físico. A tensão da fonte não é medida.

## Comandos preservados e limites

p suave; f rápido; i vai e vem TT; t vai e vem TT + normal; s normal pelo B10K; x parar/reconhecer falha; r parar/apagar sentido; z parar/zerar ACS (~1,2 s); 0/1/2/3 ACS desconhecido/5A/20A/30A; g PID; h sensores; l Monitor; v velocidade; c coleta; ? ajuda.

Proteções de posição, ausência de progresso, atraso no controle e desconexão do Serial mantidas. Não há corte térmico ou por corrente. A faixa de 180° continua nominal, sob a hipótese ADC 0–4095 = 0–216°. Nenhuma identificação da tensão máxima ou limite térmico do motor foi feita por este ajuste.

