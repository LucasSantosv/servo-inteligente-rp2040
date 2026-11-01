# Gravador V3 — temperatura e coleta completa

Abra GravadorDeTestes.exe desta pasta após extrair o ZIP. Feche o Monitor/Plotter Arduino para liberar a porta. Escolha a placa e clique Conectar. O aplicativo reconhece a programação V13, V14 ou V15 sem iniciar movimento; pede parada ao conectar.

Na V14 e V15, usa automaticamente a coleta completa: posição, PID, velocidade, RPM/pico, PWM, corrente, temperatura, estados dos sensores e tempo do RP2040. Na V13, continua usando os dados disponíveis no Plotter, sem inventar temperatura ou velocidade ausente.

Clique Suave, Rapido ou Vai e vem para iniciar. Cada clique cria o nome e salva automaticamente o novo teste. PARAR para e conclui o arquivo após mais um segundo; fechar ou desconectar também finaliza o registro. Zerar corrente inicia seu próprio registro e envia z. Se souber a versão do ACS, escolha 5/20/30 A; caso contrário, mantenha Nao identificado.

O gráfico mostra alvo e posição no eixo esquerdo e temperatura no direito. Temperatura indisponível não é desenhada como se fosse uma leitura real. O CSV também guarda P/I/D/FF e todos os outros campos transmitidos.

Tudo fica em **Documentos > ServoInteligente > Testes**. O botão Ver testes salvos abre essa pasta. Cada teste tem dados.csv, serial.txt e teste.txt; o resumo inclui maior temperatura válida observada, erro de posição, PWM e corrente observados. NTC_OK=1 indica leitura plausível, não sensor calibrado. Corrente_mA só é interpretável com Amp_OK=1. Campos vazios não são zeros.

O nome automático identifica o modo pedido e firmware detectado, não garante que houve movimento ou sucesso. Os dados só são gravados enquanto o aplicativo está aberto e conectado; o gráfico conserva aproximadamente os últimos 600 pontos, e o arquivo conserva todos os quadros recebidos durante o registro. O aplicativo lembra a última porta, mas não reinicia um movimento ao reconectar.

Para analisar com IA depois, envie dados.csv e teste.txt do teste escolhido. Não há IA nem ajuste automático de PID dentro desta janela. A fonte de alimentação não é medida: o aplicativo não sabe qual tensão você ajustou nela.

Use a V14/V15 com o esquema de multiplexador/NTC em COMECE_AQUI.md da pasta do firmware. O Gravador V3 é necessário para identificar a V15. Não há necessidade de instalar Python.

Compilado e testado com dados sintéticos, incluindo temperaturas válidas/inválidas e compatibilidade V13/V14/V15. PREVIA.png contém dados fictícios. Falta verificar comunicação com a placa e comportamento físico; nenhum motor foi acionado por esta tarefa.

