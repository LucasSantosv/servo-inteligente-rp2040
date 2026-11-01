# V11 — TT pelo B10K com PID

Abra `teste_tt_v11_b10k_pid.ino` na pasta com os três arquivos `.h`. Placa: Raspberry Pi Pico (RP2040, núcleo Arduino-Pico). Monitor Serial: 115200 baud. Esta versão foi compilada, mas ainda precisa ser gravada e testada na sua montagem.

## Teste pelo potenciômetro

1. Comece com o eixo próximo do meio do curso e o B10K no meio.
2. Envie **p**. Se necessário, ele dá pequenos pulsos para aprender o sentido; depois o TT acompanha o B10K com PID, aceleração e mudanças graduais de aceleração.
3. Gire o B10K: uma ponta pede 18°, o meio aproximadamente 108° e a outra ponta 198° na escala nominal.
4. Envie **x** para parar.

Para testar a velocidade, envie **f** e mova o B10K entre os lados. Esse modo continua usando PID de posição e libera até **255/255 de PWM (100%)** quando o erro pede. Tem resposta mais rápida e pode mudar de direção bruscamente: não usa a rampa suave do modo p. Atingir 100% não é garantido em pequenos movimentos.

Envie **v** para ver velocidade e pico. O pico também aparece continuamente em `PICO_RPM` e permanece disponível depois de x. Ele zera ao iniciar p, f, i ou t. A detecção inicial de sentido não entra no pico.

O pico é a velocidade atingida **pelo eixo acoplado ao potenciômetro, nesse percurso**, com essa alimentação e carga. Não mede o motor interno antes da redução nem garante a rotação máxima em giro livre. Não há giro contínuo com esse potenciômetro acoplado.

## Ligações mantidas

| Função | Ligação |
|---|---|
| B10K de comando | Cursor no GP26; extremos em 3V3 e GND |
| Potenciômetro de posição | Cursor no GP27; extremos em 3V3 e GND |
| Ponte MX1508, motor A | IN1 no GP6; IN2 no GP7; motor na saída A |
| Alimentação da ponte | Fonte de 5 V usada no teste; GND comum ao RP2040 |
| Servo normal opcional | Sinal GP18; alimentação externa adequada e GND comum |
| ACS712 / GP28 | Ignorado pelo programa |

As entradas analógicas recebem somente a faixa de 0 a 3,3 V.

## Comandos

| Comando | Ação |
|---|---|
| p | TT acompanha B10K suavemente com PID |
| f | TT acompanha B10K rapidamente com PID; até 100% de PWM |
| v | Velocidade e pico |
| i | Vai e vem automático do TT, preservado |
| t | Vai e vem automático TT + servo normal |
| s | Somente servo normal pelo B10K |
| x | Para e reconhece falha; selecione novamente o modo |
| r | Para e apaga o sentido aprendido; próximo modo detecta de novo |
| ? | Ajuda |

Não inicia sozinho, não reinicia sozinho após falha e para se o monitor Serial desconectar. O sentido fica na memória RAM até r ou reinicialização. Não usa marcação manual de dois pontos.

## Suavidade e leitura

- A função `seguirPotenciometro()` executa o controle do B10K a cada aproximadamente 5 ms.
- Em p, o alvo tem limite de 45°/s, aceleração de 90°/s² e variação de aceleração de 360°/s³. A posição pedida também é filtrada. Alterar o B10K preserva a continuidade do movimento; não reinicia a rampa a cada leitura.
- O corte nas pontas tem prioridade sobre a continuidade da rampa. Esses limites são do alvo; o movimento real depende do ajuste do PID, atrito, folgas, carga e alimentação.
- Em p, PWM máximo de 200/255; em f, 255/255. A detecção de sentido usa pulsos de 200/255. O vai e vem automático mantém o perfil de 20°/s da V10.
- `POS`: leitura bruta de posição; `POT`: ângulo pedido pelo B10K; `ANG`: posição estimada; `ALVO`: referência enviada ao PID.
- `VEL`: velocidade filtrada em graus/s, com sinal. `RPM`: módulo de VEL dividido por 6. `PICO_RPM`: maior módulo observado desde o início do modo.
- A estimativa de velocidade usa a diferença da posição e pode apresentar ruído; não é uma medição com encoder independente.

## Curso e paradas

A escala continua assumindo que **ADC de 0 a 4095 corresponde linearmente a 216°**, conforme o curso informado. Assim, 18° a 198° representa 180° nominais com margem. Isso não comprova 180° físicos: se o meio real indicar 216°, a escala/leitura precisa ser resolvida antes de confiar nos ângulos ou RPM.

O programa corta o comando para fora nas pontas de 18°/198° e registra falha fora da margem de 9° a 207°. Mantém a proteção de ausência de progresso e de atraso no controle. Um novo pedido do B10K renova a espera longa por movimento, sem apagar a contagem de esforço alto sem movimento.

Se parar por falha, veja o motivo no Serial, envie x e selecione p ou f após conferir a causa. O ACS não participa: esta versão não tem proteção por corrente. Os valores do PID são iniciais e ainda exigem avaliação na bancada.
