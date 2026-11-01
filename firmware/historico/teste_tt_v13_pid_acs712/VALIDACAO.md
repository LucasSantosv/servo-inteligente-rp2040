# Validação da V13

A V13 acrescenta telemetria do ACS712 no GP28 à V12. O zero é explícito pelo comando z, com o motor parado; espera 500 ms e coleta 128 médias sem delay. Novos comandos de movimento cancelam uma calibração em andamento. O sensor não altera o PID e não dispara proteção por corrente.

Testes C++ do mesmo CorrenteACS712.h entregue verificaram zero temporizado, ausência de conversão sem escala, três sensibilidades, leituras positivas/negativas de aproximadamente 1 A, ADC nas extremidades, rejeição de zero fora da faixa, rejeição de instabilidade e cancelamento do zero.

O PID, o seguidor do B10K e a conversão da ponte foram mantidos byte a byte nos três arquivos .h da V12. As 216 simulações históricas pertencem à V11 e não foram repetidas nesta alteração de medição.

Sensibilidades e alimentação conferidas no datasheet primário da Allegro. Não gravado ou testado na montagem; não houve validação física da corrente, divisor ou calibração. Não há proteção de sobrecorrente nesta versão.

Compilação concluída para rp2040:rp2040:rpipico, Arduino-Pico 6.0.0: 67.860 bytes de programa (3%) e 9.512 bytes de variáveis globais (3%). Testes do módulo ACS concluídos sem falhas. Comparação por hash confirmou os três arquivos de controle idênticos aos da V12.

