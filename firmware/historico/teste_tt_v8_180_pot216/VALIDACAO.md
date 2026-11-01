# V8 - verificacao em 31/10/2026

## Compilacao

Aprovada para rp2040:rp2040:rpipico, Arduino-Pico 6.0.0.
Programa: 63.340 bytes. Variaveis globais: 9.412 bytes.
Biblioteca Servo incluida no pacote Arduino-Pico. Versoes anteriores preservadas.

## Testes de calculo

Assercoes C++ aprovadas usando Servo216.h real do firmware:

- Percurso de 180 graus com margens simetricas no curso informado de 216.
- Pontos ADC nominais 341 e 3754; erro de quantizacao do arco inferior a 0,1 grau.
- Partida permitida nas posicoes 2574 e 3550 vistas nas capturas anteriores.
- Resposta de desaceleracao para excesso de velocidade em ambos os sentidos.
- Bloqueio de comando para fora dos limites e permissao para retornar.
- Conversao dos extremos do servo comum em 1000 e 2000 us.

## Simulacao de controle

A mesma classe Malha usada no sketch foi executada em 27 combinacoes de
planta simplificada: velocidade maxima 120/300/600 graus/s; constante de
inercia 0,03/0,08/0,15 s; limiar de atrito 0/50/90 unidades PWM.
Cada ensaio durou 65 s simulados com passos de 5 ms, quantizacao ADC e
intervalo desligado ao inverter. Todos atingiram cinco pontas, alternando
os dois lados sem sair da margem de 9 a 207 graus na simulacao.
Os extremos observados no conjunto de modelos ficaram aproximadamente
entre 15,9 e 200,1 graus. Isso mostra que o modelo ainda pode ultrapassar
o alvo; nao e garantia de ultrapassagem maxima no TT real.

Os modelos nao foram identificados a partir de medicoes do motor do usuario.
Nao representam certificado de seguranca, sintonia fisica ou comprovacao de
180 graus reais. Servem para verificar a logica antes do ensaio de bancada.

## Pendente na montagem

Sem gravacao ou teste fisico da V8. Confirmar se 216 graus e o curso eletrico
util, conferir escala angular externa, registrar velocidade/ultrapassagem e
ajustar KP/KD se necessario. Sem sensor de corrente por solicitacao do usuario.
