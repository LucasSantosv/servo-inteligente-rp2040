# V15 BANCADA-CARGA — continua controlando sob carga

## Mudança solicitada

Depois da detecção inicial do sentido, a ausência de progresso passa a ser **um aviso sem parada**. Ao segurar o eixo, ou se ele ficar atrás do alvo sob carga, o PID continua calculando e aplicando PWM. Ao voltar a haver movimento ou aproximação do alvo, o aviso se apaga automaticamente; não é necessário enviar x para reconhecer esse aviso.

Isso significa que o motor pode continuar energizado e exercendo esforço enquanto estiver imobilizado. Não foi acrescentado limite de corrente ou temperatura. Essa variante corresponde ao teste de bancada solicitado, não a uma validação para o robô definitivo.

- **SEM_MOV=1**: detector de progresso ativo como aviso.
- **ESTADO=5**: controle ativo com aviso de progresso. É um código de telemetria; a máquina de estados interna continua executando o modo escolhido.
- **FALHA=0**: nenhuma das falhas que ainda interrompem o controle está ativa.

A lógica de início sem movimento continua limitada: se o sentido ainda não foi aprendido, a detecção inicial não fica tentando indefinidamente. Os limites de posição do potenciômetro, atraso do controle, parada x, falha de sinal do servo normal e perda do Serial continuam podendo parar. Se aparecer FALHA=1, envie **l** para consultar o motivo; esta variante só remove a parada por falta de progresso durante o controle.

## Primeiro uso

1. Extraia e abra **teste_tt_v15_bancada_carga.ino**, com os seis arquivos .h juntos; grave como Raspberry Pi Pico (Arduino-Pico/RP2040).
2. Abra Monitor Serial em **115200**. Ele atualiza uma vez por segundo e mantém colunas fixas.
3. Deixe o eixo aproximadamente no meio. Envie **xz** para reconhecer eventual falha, parar e calibrar o zero do ACS. Aguarde cerca de **1,2 segundo**, sem movimentar o motor.
4. Se conhecer a variante do ACS, envie **1** para 5 A, **2** para 20 A ou **3** para 30 A. Se não souber, mantenha **0**: use ACS_mV, sem inventar amperes.
5. Envie **f** para seguir B10K rápido ou **p** para suave. **x** para. O ajuste D da V15 foi preservado: 1,0 no f e 1,8 nos outros modos.

## Corrente desaparecida / zerada

Na imagem enviada, ACS_OK=0 e Amp_OK=0. Nesse estado, os números ACS_mV e mA em zero são marcadores de leitura indisponível, não uma confirmação de corrente nula. Reiniciar ou regravar a placa perde a calibração e a seleção da escala, que estão na RAM.

Agora o Monitor também mostra:

- **ACS_ADC**: leitura bruta real do GP28, mesmo antes de calibrar.
- **ESCALA**: 0 desconhecido; 1/2/3 correspondem a 5/20/30 A.
- **ZERO**: 1 enquanto a medição do zero está em andamento.
- **ACS_OK**: zero aceito e leitura fora das extremidades ADC.
- **Amp_OK**: ACS_OK e escala selecionada. Não confirma que o modelo físico foi escolhido corretamente.

Com ACS a 5 V e divisor 10k/10k, a leitura em repouso costuma ficar perto de **1550 contagens** na hipótese ADC de 3,3 V. A plausibilidade não comprova exatidão. Após z, ACS_OK deve ficar em 1 se o sinal passar pela verificação. Se não ficar, envie os valores ACS_ADC e ZERO do Monitor para diagnóstico, antes de escolher arbitrariamente uma escala.

## Ligações preservadas

| Função | GPIO / ligação |
|---|---|
| CD74HC4067 S0/S1/S2/S3 | GP2/GP3/GP4/GP5 |
| MUX VCC / GND / EN | 3V3 / GND / GND |
| MUX SIG | GP26 |
| C0 | Cursor B10K; extremos 3V3/GND |
| C1 | Junção: 3V3 -> resistor 4,7k -> C1 -> NTC 100k -> GND |
| Posição do eixo | Cursor GP27; extremos 3V3/GND |
| ACS OUT | GP28 pelo divisor 10k/10k; ACS VCC 5 V e GND comum |
| MX1508 motor A | IN1 GP6, IN2 GP7; TT saída A |
| Servo normal opcional | Sinal GP18; fonte adequada e GND comum |

C2–C15 reservados. NTC 100k/B3950 baseado na configuração da extrusora; Temp_C só é disponível com NTC_OK=1. Nenhum sensor deve aplicar 5 V diretamente à entrada ADC ou aos canais do multiplexador.

## Plotter e gravação

**g**: PID, oito curvas. **h**: sensores, oito curvas. **l**: Monitor lento. **c**: coleta completa. O aviso SEM_MOV aparece no Monitor; no CSV da coleta ele aparece como ESTADO=5, mantendo a ordem das 28 colunas D15 e a compatibilidade com o Gravador V3. Os gráficos g/h preservam os sinais anteriores; FALHA não sobe só por falta de progresso.

O Gravador V3 existente continua compatível. Ele não tem uma mensagem específica para ESTADO=5, mas registra o código no CSV. Arquivos continuam identificados como V15; a distinção desta entrega é a variante BANCADA-CARGA.

O arco de 180° é nominal, baseado em ADC 0–4095 = 0–216°. A tensão da fonte não é medida, e o software não identifica a tensão máxima permitida pelo motor.
