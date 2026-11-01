# Validação — V15 BANCADA-CARGA

Alteração da política de ausência de progresso: as duas chamadas que antes bloqueavam o movimento durante o controle agora só atualizam AvisoProgresso. Não há chamada a parar/bloquear por esse aviso. O cálculo PID e o comando da ponte continuam em execução. O retorno ao movimento limpa o aviso; parada e preparação do controle também limpam o indicador.

Teste C++ usando os cabeçalhos entregues: posição mantida por 12 segundos com erro e PWM alto gerou aviso enquanto o PWM do PID permaneceu positivo; deslocamento da posição limpou o aviso; ausência de progresso prolongada em PWM moderado também foi sinalizada sem parada. O teste conferiu que os cortes externos de posição continuam definidos. Não substitui validação do sketch na placa.

Monitor: uma atualização por segundo, colunas fixas e diagnóstico ACS_ADC/ESCALA/ZERO mesmo antes de calibrar. Quadro de 234 bytes nos valores dentro das larguras declaradas, inferior a 256. Plotter e coleta mantêm 50 ms. Protocolo D15 continua com 28 campos; ESTADO=5 representa controle com aviso, mantendo compatibilidade com o Gravador V3.

Ainda interrompem: posição fora da margem externa, atraso do controle, falha de sinal do servo normal e ausência de movimento durante a detecção inicial de sentido. A perda do Serial para. x também para. Não existe corte por temperatura/corrente nesta variante.

Não gravado na placa e não validado fisicamente. A imagem enviada mostrava FALHA=1, mas o motivo textual da falha não foi fornecido; portanto, a causa exata da parada anterior ainda não foi comprovada. ACS_OK=0/Amp_OK=0 demonstravam leitura de corrente indisponível, sem distinguir falta de calibração de sinal inválido.

Compilação final concluída para rp2040:rp2040:rpipico, Arduino-Pico 6.0.0: 69.908 bytes de programa (3%) e 9.500 bytes de variáveis globais (3%).

