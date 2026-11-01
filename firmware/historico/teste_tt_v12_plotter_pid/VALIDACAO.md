# Validação da V12

Mudança de observabilidade sobre a V11: cópia das contribuições P/I/D/FF usadas pelo cálculo, quadros rotulados a cada 50 ms, supressão de mensagens no modo gráfico e comandos g/l para selecionar a saída.

A expressão original do PID, ganhos, limites, pinagem e proteções foram mantidos. As variáveis de diagnóstico zeram ao parar/reiniciar a malha. O quadro é descartado quando não há espaço suficiente no Serial, para evitar espera do gráfico no controle. Falhas continuam parando o motor, com flag numérica no gráfico e motivo retido para consulta pelo comando l.

O formato de saída foi conferido com a documentação oficial do Plotter Arduino. Não foi feito teste físico ou visualização com a placa conectada. Não foi gravado por esta tarefa.

As 216 simulações anteriores pertencem à V11; não foram repetidas para esta alteração de saída. Compilação V12 concluída para rp2040:rp2040:rpipico, núcleo Arduino-Pico 6.0.0: 66.580 bytes de programa (3%) e 9.448 bytes de variáveis globais (3%).

