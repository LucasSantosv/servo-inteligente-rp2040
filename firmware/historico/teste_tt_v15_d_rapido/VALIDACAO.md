# Validação V15

Compilada para rp2040:rp2040:rpipico, Arduino-Pico 6.0.0. Programa: 69.252 bytes (3%); globais: 9.500 bytes (3%). Não gravada na placa nesta tarefa.

A mudança de controle é o ganho D do modo rápido: 1,8 -> 1,0. Outros ganhos, filtro e limites mantidos. Não há um limitador visual do D.

## Testes

- 216 combinações simuladas de modo p/f, velocidade, inércia, atrito e carga: nenhuma falha nos critérios de percurso, retorno ao meio, limites externos e proteção de progresso, usando os cabeçalhos finais entregues.
- Nove modelos adicionais com velocidade livre de 600/1000/1500 graus/s e constantes de tempo de 10/30/80 ms: após oito segundos com alvo fixo, amplitude final de posição inferior a 0,1 grau nos dois segundos finais. Esses números são do modelo, não de medições do TT.
- Na referência V14, o modelo de 1500 graus/s e 30 ms oscilou aproximadamente 23,2 graus de pico a pico. Essa oscilação não persistiu com o ganho final V15 no mesmo modelo.
- 20.000 ciclos de entradas sintéticas compararam a malha não rápida V14/V15: mesmos comandos PWM, parcela D e integral. Isso verifica que o cálculo desse modo foi preservado; não testa todo o firmware embarcado.
- Reduções maiores do D não foram escolhidas porque apareceram falhas em aproximações das pontas nos testes de modelos. O valor final foi selecionado com os critérios de oscilação e percurso, sem provar adequação para qualquer motor ou carga.
- Gravador V3 compilado e exercitado com formatos V13/V14/V15, CSV, estados de sensores, números negativos, resumo, temperatura e nomes automáticos. Nenhuma porta serial foi aberta.

A leitura de sensores e as ligações vieram da V14. A saída Plotter agora tem exatamente oito campos por página (g/h); a coleta completa usa D15 com a mesma ordem de 28 campos totais do protocolo anterior. O Gravador V3 reconhece a nova identidade sem misturar nomes de versões.

Não houve teste físico, ajuste com dados gravados da montagem, validação de ruído ou confirmação de que 7,5 V é permitido pelo motor. Primeiro comparar em 5 V, sem mudar carga ou montagem. Um gráfico isolado não identifica a dinâmica completa do conjunto.
