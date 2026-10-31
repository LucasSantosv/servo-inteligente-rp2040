# V7 - verificacao em 31/10/2026

Compilacao aprovada: rp2040:rp2040:rpipico, Arduino-Pico 6.0.0.
Codigo 63.380 bytes; RAM global 9.168 bytes.

Testes em tempo de compilacao aprovados usando as mesmas funcoes do firmware:
B isolado em 3550 nao seleciona modo manual; nenhuma referencia tambem permite
modo automatico; POS3550 e aceito; janela automatica fica centrada na leitura
atual; limites e sentido sao verificados; modo manual completo permanece.

Revisao estatica da entrada: i/t usam A/B manuais somente quando ambos sao
validos; caso contrario, calculam os limites automaticos +/-160 ADC. r limpa
as referencias. Telemetria automatica usa percentual para nao sugerir um
angulo fisico que nao foi medido.

Nao houve gravacao ou teste fisico da V7. O erro de usabilidade da V6 foi
reproduzido pelas condicoes da captura: referencia manual ausente/invalida.
Os testes nao substituem verificar o acoplamento e o movimento na bancada.
