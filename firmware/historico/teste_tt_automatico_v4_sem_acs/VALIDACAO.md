# V4 sem ACS712

Compilacao aprovada: rp2040:rp2040:rpipico, Arduino-Pico 6.0.0.
Codigo: 61.500 bytes. Variaveis globais: 9.128 bytes.

Removidos: leitura GP28, calibracao de zero, validacao de tensao do ACS,
corte por corrente e campo ACS do Serial. GP28 nao e configurado nem lido.
i/t iniciam diretamente a deteccao de sentido a partir da posicao atual.

Revisao estatica: referencia inicial de posicao registrada antes de acionar,
estado de deteccao inicializado diretamente, x/falhas ainda desligam ambos.
ControleTT.h identico ao da V3, mantendo partida em 78% por ate 50 ms,
pausa de 35 ms, janela e margens, deteccao de movimento contrario e
acompanhamento em 25%. Limite total de deteccao: 200 ms.

Nao houve gravacao ou teste fisico. Esta versao nao possui corte por corrente.
Variantes anteriores preservadas. Curso angular nao calibrado.
