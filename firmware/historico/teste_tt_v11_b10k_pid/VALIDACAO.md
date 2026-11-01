# Validação da V11

## Verificado por simulação

Teste C++ usando os mesmos arquivos `Servo216.h`, `ControleB10K.h` e `PonteMX1508.h` entregues:

- 216 combinações de modo p/f, velocidade da planta, inércia, atrito e carga: nenhuma falha nos critérios de percurso, aproximação das duas pontas, retorno ao meio, margem de posição e ausência de falsa parada.
- Cada combinação simulou 60 segundos, com comandos para uma ponta, outra ponta e meio.
- 20.000 passos de mudanças aleatórias do B10K verificaram limites da referência de posição, velocidade, aceleração e deslocamento por ciclo; também foi verificada a convergência para uma ponta.
- O modo rápido permitiu alcançar PWM de 255; a conversão desse comando para os níveis da ponte foi conferida.
- Mapeamento B10K 0–4095 para 18°–198° e conversão de graus/s para RPM conferidos.

Durante o teste foi corrigida uma falsa parada: um novo comando após repouso podia herdar o temporizador longo de ausência de movimento. Agora mudanças de pedido superiores a 1° renovam essa espera, preservando a referência de movimento e a contagem de esforço alto sem movimento.

Esses testes usam modelos simplificados do motor. Não comprovam o ajuste do PID, o curso físico, corrente, força ou velocidade real da montagem.

## Compilação

Compilação concluída para `rp2040:rp2040:rpipico` pelo Arduino CLI, utilizando o núcleo Arduino-Pico 6.0.0 e sua biblioteca Servo. Resultado: 65.900 bytes de programa (3%) e 9.416 bytes de variáveis globais (3%).

A primeira tentativa encontrou um arquivo de compilação temporário truncado. A compilação foi refeita em uma pasta nova e terminou com sucesso.

## Bancada

Não gravado por esta tarefa. V11 ainda não validada fisicamente. Não há medição de corrente, e a calibração de 216° continua sendo uma hipótese do mapeamento analógico. A velocidade exibida é derivada dessa mesma hipótese e do potenciômetro.
