# Validação — variante V15 Monitor lento

Alterações: saída inicial em Monitor (em vez de Plotter), intervalo do Monitor de 100 para 1000 ms e campos numéricos com largura fixa. O texto do Monitor inclui P/I/D/FF e mantém o prefixo V15 POS= e valores imediatamente após os sinais de igual; o preenchimento vem depois dos valores, preservando o formato reconhecido pelo Gravador V3.

Não houve mudança de malha PID, ganhos, perfil, pinagem, proteções, leitura de sensores ou ordem dos campos da coleta D15. Plotter e coleta permanecem com intervalo de 50 ms. Não foram repetidas as simulações de controle porque o controle não foi alterado.

Não gravado na placa nesta tarefa. Falta verificar a apresentação na janela da IDE e a comunicação física. A IDE ainda acrescenta linhas: a alteração fixa as colunas, não redesenha uma tela estática.

Compilação concluída para rp2040:rp2040:rpipico, Arduino-Pico 6.0.0: 69.332 bytes de programa (3%) e 9.500 bytes de variáveis globais (3%). Os cinco cabeçalhos de controle/sensores foram comparados por hash e são idênticos aos da V15 anterior. O quadro formatado ocupa 195 bytes para valores dentro das larguras declaradas.

