# Validação atual e histórico

## Firmware V15 BANCADA-CARGA

[Notas da revisão](../firmware/atual/teste_tt_v15_bancada_carga/VALIDACAO.md). Compilação já verificada nesta máquina para rp2040:rp2040:rpipico, Arduino-Pico 6.0.0: 69.908 bytes de programa e 9.500 bytes globais. Nesta publicação não houve gravação de placa pelo agente; o autor executou testes e forneceu imagens.

As 216 combinações simuladas e os nove modelos de resposta rápida descritos na V15 dizem respeito ao ajuste do PID anterior à política de aviso. O teste atual de bancada exercita aviso sob ausência de progresso e recuperação, sem interromper o PWM. Os testes não certificam montagem, tensão, carga, ruído, vida útil nem curso físico.

```sh
arduino-cli compile --fqbn rp2040:rp2040:rpipico firmware/atual/teste_tt_v15_bancada_carga
```

## Testes C++ no computador

Usam os cabeçalhos do repositório. As referências de inclusão foram ajustadas aos caminhos publicados, e os testes foram executados novamente após essa preparação.

```sh
g++ -std=c++17 -O2 tests/carga_v15_test.cpp -o carga_test
./carga_test
g++ -std=c++17 -O2 tests/b10k_v15_test.cpp -o b10k_test
./b10k_test
g++ -std=c++17 -O2 tests/ntc_test.cpp -o ntc_test
./ntc_test
```

O teste B10K inclui o detector original de progresso como parte do modelo. A política de não bloquear é verificada separadamente por carga_v15_test, que usa AvisoProgresso. Os pontos do NTC são dados teóricos gerados pela própria curva Beta; isso verifica software e quantização, não calibração do sensor.

## Gravador V3

Código C# e executável para Windows em tools/GravadorDeTestesV3. Compilado, testado com dados sintéticos V13/V14/V15 e janela renderizada com prévia fictícia. O modo de autoteste aceita `--testar` seguido de uma pasta de saída. Ele não abre a porta serial. Não houve validação física do gravador com o motor neste registro.

## Evidência física

O autor informou cinco minutos de teste extremo a 7,5 V e forneceu capturas com máxima visível de 51,4 °C. Isso é separado dos testes simulados e da compilação. [Condições, imagens e limitações](ensaio-termico-2026-10-31.md). Não há CSV integral ou medição de corrente em amperes para esse ensaio.

## Versões anteriores

Notas de cada revisão foram preservadas em suas pastas. A V7 publicada anteriormente continua no caminho antigo para manter links e o teste curso_v7_test; não é a versão atual. Cada nota descreve a etapa em que foi escrita. Registros antigos de teste físico pendente não devem ser confundidos com a situação atual, nem o teste atual valida retroativamente todos os códigos anteriores.
