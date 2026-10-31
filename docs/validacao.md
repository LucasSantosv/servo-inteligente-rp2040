# Validação

## Firmware atual

V7 compilada em 31/10/2026 com Arduino-Pico 6.0.0, alvo
`rp2040:rp2040:rpipico`: 63.380 bytes de programa e 9.168 bytes de variáveis
globais. Biblioteca Servo fornecida pelo pacote Arduino-Pico. Não houve flash
ou teste físico realizado pelo agente.

Os testes em C++ são `static_assert` e usam as funções reais de `Curso180.h`.
Cobrem o caso observado de B isolado em 3550, ausência de pontos, início em
POS3550, criação de janela automática, corte nos extremos e sentidos do motor.
Não simulam a inércia, o ADC real, o acoplamento ou a malha completa.

No Arduino IDE, abra a pasta do sketch atual. Pela linha de comando:

```sh
arduino-cli compile --fqbn rp2040:rp2040:rpipico firmware/atual/teste_tt_v7_auto_sem_marcacao
```

O teste de regras pode ser compilado com um compilador C++17:

```sh
g++ -std=c++17 -fsyntax-only tests/curso_v7_test.cpp
```

Verificar uma revisão antiga requer abrir seu próprio sketch em
`firmware/historico/`. As notas de validação dessas pastas dizem respeito à
respectiva revisão, não à versão atual.

## Próximo teste de bancada

Enviar i sem marcar A/B e confirmar a mensagem V7 AUTO. Observar se o retorno
acompanha o movimento, se há inversão nos dois lados e qual mensagem aparece
em caso de corte. Registrar o resultado e a revisão gravada. Só ampliar curso
ou ajustar o controle após observar esse ensaio.
