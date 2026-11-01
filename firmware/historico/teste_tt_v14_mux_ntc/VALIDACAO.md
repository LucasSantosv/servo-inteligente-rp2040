# Validação da V14

Compilação concluída para rp2040:rp2040:rpipico com Arduino-Pico 6.0.0: 69.148 bytes de programa (3%) e 9.496 bytes de variáveis globais (3%). Não gravado na placa nesta tarefa.

Comparação por hash confirmou Servo216.h, ControleB10K.h, PonteMX1508.h e CorrenteACS712.h idênticos aos da V13. A integração acrescenta o multiplexador, descarte da primeira conversão ADC, conversão/filtro NTC e telemetria; mantém ganhos e proteções anteriores do controle.

Teste C++ do mesmo TemperaturaNTC.h verificou pontos sintéticos em -20, 0, 25, 50, 75, 100 e 125 °C, filtro, detecção de leitura extrema/curto/aberto e recuperação após reconexão. Os pontos foram gerados pela curva teórica, portanto testam a matemática e a quantização, não o sensor físico.

Formatos de telemetria extraídos do próprio sketch foram exercitados com valores extremos de teste: coleta completa 145 bytes/28 campos, Monitor 186 bytes e Plotter 156 bytes. Todos ficaram abaixo de 256 bytes nos cenários exercitados; quadros que não couberem no USB são descartados para evitar espera no controle.

O Gravador V2 foi compilado e testado com quadros V13, V14 e D14: mapeamento de campos, validade da temperatura, rejeição de quadros incompletos/não finitos, CSV em pt-BR, resumo e nomes únicos. A janela foi renderizada e conferida com dados fictícios. Não foi aberta uma porta serial nestes testes.

Não houve validação física de multiplexador, ruído, tempo de assentamento, NTC, coeficiente Beta, aderência térmica ou operação do motor. Nenhum resultado simulado comprova temperaturas ou tensão admissível. Não existe corte por temperatura/corrente nesta entrega.
