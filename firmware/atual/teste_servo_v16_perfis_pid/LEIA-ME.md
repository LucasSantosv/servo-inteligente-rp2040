# V16 — perfis de PID pelo Monitor Serial

Variante da V15 `teste_tt_v15_bancada_carga`, sem sobrescrever a base. Feita para o motor DC de um servo convertido, controlado pela ponte MX1508 e com retorno de posição. O servo normal comandado por GP18 continua usando o controlador interno dele: estes ganhos não alteram esse servo normal.

## Usar

1. Abra `teste_servo_v16_perfis_pid.ino` com os arquivos `.h` na mesma pasta e grave na mesma placa RP2040 usada na V15.
2. Monitor Serial a 115200. Envie `f` para seguir o B10K rápido, ou `p` para seguir com rampa suave.
3. Envie um único número de `0` a `9` e Enter para trocar de perfil. Pode trocar durante o controle. Selecionar um perfil parado não inicia o motor.
4. `k` mostra os ganhos selecionados e retorna ao Monitor; `g` abre a saída do Plotter PID. O perfil também aparece em cada linha do Monitor de 1 Hz.
5. `x` para. `0` restaura os ganhos originais da V15, sem iniciar o motor.

## Perfis

| Número | Kp | Ki | Kd | Comparação |
|---|---:|---:|---:|---|
| 0 | 9 | 6 | 1,8 em p/i/t; 1,0 em f | Ganhos originais V15; padrão ao ligar |
| 1 | 4,5 | 0 | 0 | P menor |
| 2 | 9 | 0 | 0 | P intermediário |
| 3 | 13,5 | 0 | 0 | P maior |
| 4 | 9 | 0 | 0,5 | PD com D menor |
| 5 | 9 | 0 | 1,0 | PD com D intermediário |
| 6 | 9 | 0 | 1,8 | PD com D maior |
| 7 | 9 | 3 | 0 | PI |
| 8 | 9 | 3 | 1,0 | PID com I menor |
| 9 | 9 | 6 | 1,0 | PID com I maior; igual ao perfil 0 no modo f |

Nos perfis 1 a 9, Kd não muda entre os modos. Os números são combinações de teste, não níveis crescentes de potência nem sintonia validada para este motor.

P reage ao erro de posição; I acumula a correção de erro persistente; D amortece a diferença entre velocidade desejada e medida. No modo f a velocidade da referência usada pelo D é zero, portanto D = -Kd × velocidade medida. Os valores P/I/D do Plotter são as parcelas de saída, não os coeficientes Kp/Ki/Kd.

Para comparar uma parte por vez: 1/2/3 variam P; 2/4/5/6 variam D; 2/7 variam I sem D; 5/8/9 variam I com o mesmo D. Mantenha tensão, carga e modo p/f iguais durante cada comparação.

FF (compensação de atrito/velocidade), filtros, rampas, limites e pinagem foram mantidos. Mesmo em perfis P sem I/D, a parcela FF continua ativa. O ganho não altera o teto de PWM: 255 em f; 200 nos demais modos. A detecção inicial de sentido mantém o comportamento da V15.

Na troca, a memória integral é zerada para não carregar o ensaio anterior. Velocidade filtrada e rampa de saída são preservadas. Ainda pode haver mudança de esforço ao mudar ganhos. O perfil permanece nos comandos de parada/início, mas volta a 0 ao reiniciar a placa; não é salvo em flash.

## Sensores e outros comandos

Os números antes usados pelo ACS agora selecionam PID. Para escolher a variante do ACS, envie os dois caracteres juntos:

- `a0`: variante desconhecida, somente mV.
- `a1`: ACS712 de 5 A.
- `a2`: ACS712 de 20 A.
- `a3`: ACS712 de 30 A.
- `z`: para e mede o zero do ACS, como antes.

`p`, `f`, `i`, `t`, `s`, `x`, `r`, `g`, `h`, `l`, `v`, `c` e `?` mantêm as funções da V15. `l` também mostra os ganhos. Nenhuma mudança para INA219 ou encoder I2C foi feita: essa era uma discussão de arquitetura futura.

O pacote `c` mantém `D15` e as mesmas 28 colunas para compatibilidade com a coleta anterior. Esse pacote não inclui o número do perfil; o nome do ensaio precisa identificar o perfil se usar o gravador antigo. O Plotter mantém as oito curvas anteriores, sem inserir texto ao trocar de perfil.

A escala continua nominal: ADC 0–4095 corresponde a 0–216°, com alvo 18–198° e corte externo 9–207°. Trocar o motor TT por um servo não calibra automaticamente o curso do potenciômetro novo.

Ausência de progresso continua gerando aviso sem desligar, como na variante de bancada de origem. Proteções existentes não foram removidas. Não há corte por temperatura/corrente nessa base.

## Validação

Consulte `VALIDACAO.md` para os checks efetivamente concluídos. Os novos perfis precisam de teste físico; não foi feita gravação na placa por este agente.
