# Verificação da V16

- Compilação concluída com Arduino-Pico 6.0.0, alvo `rp2040:rp2040:rpipico`: 70.612 bytes de programa e 9.512 bytes de variáveis globais.
- Conferida a preservação byte a byte de ControleB10K.h, PonteMX1508.h, CorrenteACS712.h, TemperaturaNTC.h e AvisoProgresso.h em relação à V15 de bancada.
- Revisada a seleção serial de um dígito 0–9, o prefixo `a` para a escala do ACS e a manutenção de `x` mesmo após prefixo incompleto.
- Revisado que Ki=0 impede tanto integração de erro quanto acúmulo pela realimentação de saturação. Assim os perfis sem I realmente mantêm parcela integral zero.
- A troca de perfil não chama a partida do motor e preserva velocidade filtrada/rampa de saída; integral é reiniciada.
- Protocolo de coleta D15 de 28 campos e oito curvas do Plotter preservados; Monitor passa a identificar V16/PERFIL.
- Não foram realizados gravação, ensaio físico ou validação da estabilidade dos novos ganhos com o servo convertido. A compilação não comprova a resposta mecânica.
