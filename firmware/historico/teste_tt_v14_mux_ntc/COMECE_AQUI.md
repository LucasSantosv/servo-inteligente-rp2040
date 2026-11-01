# V14 — multiplexador, PID, ACS712 e NTC

## Começar

1. Monte a pinagem abaixo com a alimentação desligada.
2. Abra **teste_tt_v14_mux_ntc.ino** mantendo os cinco arquivos .h na mesma pasta e grave como Raspberry Pi Pico (Arduino-Pico/RP2040).
3. Abra o Monitor ou Plotter Serial em **115200**. Com o motor parado e o eixo no meio, confira **Temp_C** e **NTC_OK**. A leitura é considerada disponível quando **NTC_OK=1**. Compare a primeira leitura com um termômetro de referência antes de usá-la para avaliar aquecimento.
4. Envie **z** e aguarde cerca de 1,2 s para medir o zero do ACS, como na V13. Se conhecer o modelo, selecione **1** para 5 A, **2** para 20 A ou **3** para 30 A; sem identificação, mantenha **0**.
5. Envie **p** para seguir o B10K suavemente, **f** para modo rápido ou **i** para vai e vem. **x** para.

A V13 e o gravador anterior foram preservados. A V14 exige o B10K no C0 do multiplexador: não é a mesma ligação direta da V13.

## Pinagem

Números GP são os GPIOs, não a posição física do conector.

| Componente | Ligação |
|---|---|
| MUX CD74HC4067 VCC | 3V3 |
| MUX GND | GND comum |
| MUX EN | GND |
| MUX S0 | GP2 |
| MUX S1 | GP3 |
| MUX S2 | GP4 |
| MUX S3 | GP5 |
| MUX SIG/COM | GP26 |
| MUX C0 | Cursor do B10K; extremos do B10K em 3V3/GND |
| MUX C1 | Ponto central do divisor NTC abaixo |
| MUX C2 a C15 | Reservados para futuras entradas; ainda não são lidos |
| Potenciômetro de posição | Cursor GP27; extremos 3V3/GND |
| ACS712 | OUT pelo divisor 10k/10k ao GP28; VCC 5 V e GND comum |
| MX1508 motor A | IN1 GP6, IN2 GP7; TT na saída A |
| Servo normal opcional | Sinal GP18; alimentação externa adequada e GND comum |

**S1/S2/S3 não ficam no GND: agora vão aos GP3/GP4/GP5.**

```text
3V3 ── resistor FIXO 4,7k ──┬── C1 do multiplexador
                           │
                        NTC 100k
                           │
                          GND
```

Repete a configuração do sensor da extrusora: NTC de **100k a 25 °C, Beta 3950**, resistor fixo de **4,7k**. É uma hipótese baseada no código da extrusora e na compra em conjunto, ainda não uma identificação física comprovada. Não trocar o resistor fixo por 100k sem mudar R_FIXO em TemperaturaNTC.h.

Todos os canais do multiplexador devem permanecer entre GND e 3V3. O OUT do ACS não pode ir direto ao GP28 ou ao multiplexador. O positivo da fonte do motor passa por ACS IP+ -> IP− -> positivo da ponte; negativo da fonte, ponte e RP2040 compartilham GND. O sensor mede a alimentação da ponte, não toda a corrente que recircula pelo enrolamento.

Prenda o corpo sensível do NTC junto à carcaça metálica do motor, com bom contato térmico e isolamento elétrico dos terminais. Ele mede a superfície, com atraso em relação ao enrolamento interno.

## Temperatura

- O B10K é lido pelo C0 em cada ciclo de controle (~5 ms).
- O NTC é lido pelo C1 a cada 100 ms (10 leituras/s), inclusive com o motor parado.
- Após selecionar um canal, o programa espera 100 microssegundos e descarta a primeira conversão ADC antes da média. Depois de ler C1, volta a selecionar C0. Posição e corrente continuam diretas nos GP27/GP28.
- A conversão usa a equação Beta. A temperatura válida recebe filtro de aproximadamente 0,5 s. Sensor inválido zera a saída e baixa NTC_OK; o zero nesse estado não significa 0 °C.
- NTC_OK só indica leitura plausível para o divisor/curva configurados. Não confirma aderência ao motor, precisão, valor Beta ou calibração.
- NTC_estado no gravador: 0 aguardando, 1 leitura plausível, 2 ADC próximo de zero/curto, 3 ADC próximo do máximo/aberto, 4 cálculo fora da faixa. Causas físicas diferentes podem produzir os mesmos códigos.

Esta versão mede e registra temperatura, **sem corte por temperatura ou corrente**. Não foi escolhido um limite térmico específico para esse motor. As proteções de posição, falta de progresso, atraso de controle e perda do Serial foram mantidas. A faixa plausível de cálculo (-30 a 150 °C) não é uma faixa de operação permitida para o motor.

## IDE Arduino

O gráfico é o modo inicial. **g** seleciona o Plotter: Alvo, Posicao, P, I, D, FF, PWM, Falha, ACS_mV, Corrente_mA, ACS_OK, Amp_OK, **Temp_C** e **NTC_OK**. Desmarque curvas para facilitar a escala compartilhada. **l** seleciona o Monitor, incluindo Temp_C, NTC_OK e NTC_ADC. **v** mostra velocidade e pico no Monitor.

## Gravação automática

Use **GravadorDeTestesV2**, que reconhece V13 e V14. Feche o Monitor/Plotter antes de conectar pelo gravador. Na V14, ele seleciona automaticamente o novo modo **c**, que envia todos os campos juntos: PID, posição, velocidade, RPM/pico, PWM, corrente e temperatura, com tempo em milissegundos do RP2040. Não precisa digitar c ao usar a janela.

Os nomes dos testes continuam automáticos. O CSV e o resumo passam a incluir temperatura e sua validade. Os dados ficam em Documentos > ServoInteligente > Testes. O gravador V1 não identifica V14.

## Comandos preservados

p suave B10K; f rápido B10K; i vai e vem TT; t vai e vem TT + normal; s normal pelo B10K; x parar/reconhecer falha; r parar/apagar sentido aprendido; z parar/zerar ACS; 0/1/2/3 selecionar ACS desconhecido/5A/20A/30A; g Plotter; l Monitor; v velocidade; c coleta completa; ? ajuda.

O modo p mantém os limites anteriores do alvo (45°/s, 90°/s² e variação de aceleração de 360°/s³), PWM até 200/255. f mantém até 255/255. O arco nominal continua 18–198° sob a hipótese ADC 0–4095 = 0–216°. Os ganhos do PID não foram alterados.

A tensão da fonte não é medida por este circuito. Temperatura e corrente não identificam sozinhas uma tensão máxima permitida para o motor; comparações de tensão precisam também do valor real da fonte e das condições de carga.
