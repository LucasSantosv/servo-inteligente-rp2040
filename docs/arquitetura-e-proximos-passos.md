> Documento de arquitetura da etapa inicial. Para a montagem física e os resultados atuais (V15, multiplexador e NTC), consulte o README da raiz e o relatório térmico. As opções de arquitetura/PCB abaixo continuam propostas, não decisões fechadas.

# Arquitetura e próximos passos

## Protótipo atual

Um RP2040 lê o potenciômetro do eixo e aciona uma MX1508 para movimentar um
TT. O B10K separado controla o servo comum no modo s. O ACS712 foi usado nos
primeiros testes e removido do controle por solicitação do autor. Não houve
calibração de corrente. O conjunto ainda é experimental.

## Decisões em aberto

| Opção | Efeito a avaliar |
|---|---|
| Um controlador por atuador | Controle e parada locais; repete MCU, regulador e comunicação. Facilita modularidade, mas aumenta componentes. |
| Controlador central | Compartilha MCU e parte da alimentação; exige mais entradas analógicas, saídas e cabeamento. Os canais ADC disponíveis não bastam diretamente para 20 retornos. |
| Controladores por grupo de motores | Alternativa intermediária; ainda depende do número de eixos e da comunicação. |

O ensaio com um RP2040 e um motor não decide a arquitetura final. Antes de
um lote, é preciso definir corrente, torque, tensão, curso, velocidade,
repetibilidade, dimensões e comportamento diante de perda de comunicação.

## Componentes considerados

- **MX1508:** ponte disponível usada nesta bancada. Capacidade térmica e de
  corrente precisam de medição para o motor escolhido.
- **DRV8876:** alternativa planejada com recursos de medição/regulação de
  corrente; não foi montada neste teste. Não substitui a MX1508 pino a pino.
- **Potenciômetro:** retorno atual. Resistência não determina curso angular;
  o modelo/curso e a qualidade do acoplamento ainda precisam ser confirmados.
- **Encoder magnético de 360°:** possível evolução; sensor, ímã, interface e
  montagem ainda não selecionados. Não foi usado nas fotos.
- **SN74HC595N:** expansão de saídas digitais. Não aciona diretamente o motor
  nem mede o potenciômetro. Usá-lo para controle temporal requer projeto próprio.
- **74HC165D:** expansão de entradas digitais. Não converte sinais analógicos.
- **PID:** previsto como estudo; a versão atual usa alvo gradual e correção
  por posição, com PWM ligado/desligado e sem PID sintonizado.

## Custo e futura PCB para 20 unidades

Não há orçamento fechado. Para comparar alternativas, cotar MCU/placa,
ponte H, sensor, alimentação, conectores, passivos, PCB, montagem, frete e
retrabalho. Custo do lote = custos fixos + 20 × custo variável por unidade;
peças já disponíveis não tornam a fabricação futura gratuita.

Antes de gerar esquemático final ou Gerbers: validar um canal com medições,
estabelecer corrente de partida e travamento, escolher proteção e regulador,
testar retorno e perdas mecânicas, e revisar layout térmico e de potência.
Em seguida fabricar um pequeno piloto antes de encomendar 20 unidades.
Nenhum esquema elétrico final ou PCB foi validado nesta etapa.

