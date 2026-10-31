# V6 - verificacao

Compilacao final aprovada: rp2040:rp2040:rpipico, Arduino-Pico 6.0.0.
Codigo: 62.828 bytes. Variaveis globais: 9.144 bytes.

Testes em tempo de compilacao de Curso180.h aprovados: calibracao crescente
e invertida, conversao dos pontos fisicos em 0 e 180 graus, partida no meio,
ultrapassagem do curso, quatro combinacoes de motor/retorno, corte por sentido
contrario e conversao dos extremos do servo normal em 1000 e 2000 us.
O teste detectou um arredondamento de 1 us no extremo do servo normal;
corrigido e recompilado com os testes aprovados.

Revisao estatica: a/b param e registram pontos; i/t exigem os dois pontos e
eixo no meio; deteccao limitada a um segundo; transicao imediata para controle;
alvo alterna 0/180 indefinidamente com pausa de 300 ms; x/falhas/desconexao
desligam o acionamento. ACS ignorado, GP28 sem uso.

Usuario confirmou que consegue marcar dois pontos a meia volta de distancia.
A exatidao de 180 graus depende dessa medicao fisica e do acoplamento informado
como direto. Tolerancia de posicao: 1,5 grau; alvo 20 graus/s, sem garantia de
velocidade instantanea. Controle ainda precisa de ajuste fisico.

Nenhuma gravacao, teste de bancada, medicao de inercia ou validacao mecanica.
Versoes anteriores preservadas.
