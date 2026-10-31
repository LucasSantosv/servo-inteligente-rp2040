#include <Arduino.h>
#include <Servo.h>
#include <math.h>
#include "ControleTT.h"

// Pinagem mantida. GP sao numeros GPIO, nao posicoes fisicas do conector.
constexpr uint8_t B10K = 26, POSICAO = 27, ACS712 = 28;
constexpr uint8_t IN1 = 6, IN2 = 7, SINAL_SERVO = 18;
constexpr float LIMITE_ACS_OUT_V = 0.05f; // Sem presumir variante do ACS712
enum class Estado { Parado, ZeroACS, Detectando, Acomodando, Seguindo };
Estado estado = Estado::Parado;
Servo servo;
bool normalAtivo = false, falha = false;
int centro = 0, pos = 0, pot = 0, adcACS = 0, polaridade = 0;
int inicioDeteccao = 0, referenciaSentido = 0, esperado = 0, pwmAtual = 0;
int posAnterior = 0, referenciaProgresso = 0;
float alvoSuave = 0, zeroACS = 0, deltaACS = 0;
uint32_t inicioEstado = 0, ultimaLeitura = 0, ultimaTela = 0;
uint32_t ultimoServo = 0, inicioProgresso = 0, tempoPulsos = 0;
uint32_t somaZero = 0;
uint16_t amostrasZero = 0;
uint8_t excesso = 0;

int media(uint8_t pino) {
  int soma = 0;
  for (int n = 0; n < 8; ++n) soma += analogRead(pino);
  return soma / 8;
}

void motor(int pwm) {
  pwm = constrain(pwm, -ControleTT::PWM, ControleTT::PWM);
  if (pwm >= 0) { analogWrite(IN2, 0); analogWrite(IN1, pwm); }
  else { analogWrite(IN1, 0); analogWrite(IN2, -pwm); }
  pwmAtual = pwm;
}

void parar() {
  motor(0);
  estado = Estado::Parado;
  normalAtivo = false;
  if (servo.attached()) servo.detach();
  pinMode(SINAL_SERVO, OUTPUT); digitalWrite(SINAL_SERVO, LOW);
  esperado = 0;
  excesso = 0;
}

void bloquear(const char* motivo) {
  parar(); falha = true;
  Serial.print("PAROU: "); Serial.println(motivo);
  Serial.println("Confira a montagem. x para reconhecer; nao reinicia sozinho.");
}

void iniciar(bool usarNormal, bool usarTT) {
  parar();
  if (falha) { Serial.println("Confira a causa e envie x antes de novo teste."); return; }
  pos = media(POSICAO); pot = media(B10K);
  if (usarTT && !ControleTT::partidaValida(pos)) {
    Serial.println("TT desligado: deixe o potenciometro de posicao perto do meio e tente i/t.");
    return;
  }
  if (usarNormal) {
    servo.attach(SINAL_SERVO, 1000, 2000, map(pot, 0, 4095, 1000, 2000));
    if (!servo.attached()) { bloquear("Falha no sinal do servo normal."); return; }
    normalAtivo = true;
  }
  if (usarTT) {
    centro = posAnterior = pos;
    polaridade = 0;
    inicioEstado = millis();
    somaZero = 0; amostrasZero = 0;
    estado = Estado::ZeroACS;
    Serial.println("Configurando automaticamente. Aguarde o pequeno movimento.");
  } else Serial.println("Servo normal seguindo o B10K. x para parar.");
}

void comando(char c) {
  if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  if (c == 'x') { parar(); falha = false; Serial.println("Parado."); }
  else if (c == 's') iniciar(true, false);
  else if (c == 'i') iniciar(false, true);
  else if (c == 't') iniciar(true, true);
  else if (c == '?') Serial.println("s: servo normal | i: TT automatico | t: os dois | x: parar");
}

void atualizar(uint32_t agora) {
  pot = media(B10K); pos = media(POSICAO); adcACS = media(ACS712);
  if (normalAtivo && agora - ultimoServo >= 20) {
    ultimoServo = agora;
    servo.writeMicroseconds(map(pot, 0, 4095, 1000, 2000));
  }
  if (estado == Estado::Parado) return;
  if (!ControleTT::dentro(pos, centro) || abs(pos - posAnterior) > 40) {
    bloquear("Posicao fora da pequena faixa de teste ou retorno instavel."); return;
  }
  posAnterior = pos;
  if (estado == Estado::ZeroACS) {
    // Motor desligado. Aguarda estabilizacao e amostra o ACS automaticamente.
    if (agora - inicioEstado < 150) return;
    somaZero += adcACS; ++amostrasZero;
    if (amostrasZero < 64) return;
    zeroACS = somaZero / (float)amostrasZero;
    float volts = zeroACS * 3.3f / 4095;
    if (volts < 0.95f || volts > 1.55f) { bloquear("Confira ACS712 e divisor 10k/10k."); return; }
    inicioDeteccao = pos;
    inicioEstado = agora;
    estado = Estado::Detectando;
    return;
  }
  deltaACS = fabsf(adcACS - zeroACS) * 3.3f / 4095 * 2.0f;
  excesso = deltaACS > LIMITE_ACS_OUT_V ? excesso + 1 : 0;
  if (excesso >= 3) { bloquear("ACS acima do limite inicial de teste."); return; }
  if (estado == Estado::Detectando) {
    int delta = pos - inicioDeteccao;
    if (abs(delta) >= 12) {
      motor(0);
      polaridade = ControleTT::sinal(delta);
      inicioEstado = agora; estado = Estado::Acomodando;
    } else if (agora - inicioEstado >= 200) {
      bloquear("Nao detectei movimento suficiente. Nao aumentei a potencia.");
    } else motor(ControleTT::pulso(agora - inicioEstado) ? ControleTT::PWM : 0);
    return;
  }
  if (estado == Estado::Acomodando) {
    motor(0);
    if (agora - inicioEstado < 100) return;
    // Confirma que o deslocamento liquido ainda tem o sentido identificado.
    if ((pos - inicioDeteccao) * polaridade < 8) {
      bloquear("Retorno de posicao nao confirmou o sentido."); return;
    }
    alvoSuave = pos;
    referenciaSentido = referenciaProgresso = pos;
    inicioProgresso = tempoPulsos = agora;
    estado = Estado::Seguindo;
    Serial.println("Sentido detectado. Gire o B10K devagar. x para parar.");
    return;
  }
  if (ControleTT::contrario(referenciaSentido, pos, esperado)) {
    bloquear("Motor girou no sentido contrario ao esperado."); return;
  }
  if ((pos - referenciaSentido) * esperado > 0) referenciaSentido = pos;
  float desejado = ControleTT::alvo(pot, centro);
  alvoSuave += constrain(desejado - alvoSuave, -0.08f, 0.08f); // alvo: ate 40 ADC/s
  float erro = alvoSuave - pos;
  if (fabsf(erro) <= 8) {
    motor(0); esperado = 0;
    referenciaSentido = referenciaProgresso = pos;
    inicioProgresso = agora;
    return;
  }
  int novoSentido = erro > 0 ? 1 : -1;
  if (novoSentido != esperado) {
    esperado = novoSentido;
    referenciaSentido = referenciaProgresso = pos;
    inicioProgresso = tempoPulsos = agora;
  }
  motor(ControleTT::pulso(agora - tempoPulsos) ? esperado * polaridade * ControleTT::PWM : 0);
  if (abs(pos - referenciaProgresso) >= 8) {
    referenciaProgresso = pos; inicioProgresso = agora;
  } else if (agora - inicioProgresso > 250) bloquear("Motor sem progresso de posicao.");
}

void setup() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  analogWriteRange(255); analogWriteFreq(20000);
  pinMode(SINAL_SERVO, OUTPUT); digitalWrite(SINAL_SERVO, LOW);
  analogReadResolution(12);
  pinMode(B10K, INPUT); pinMode(POSICAO, INPUT); pinMode(ACS712, INPUT);
  Serial.begin(115200); motor(0);
}

void loop() {
  if (!Serial && (estado != Estado::Parado || normalAtivo)) parar();
  for (int n = 0; n < 16 && Serial.available(); ++n) comando(Serial.read());
  uint32_t agora = millis();
  if (agora - ultimaLeitura >= 2) { ultimaLeitura = agora; atualizar(agora); }
  if (Serial && agora - ultimaTela >= 500 && Serial.availableForWrite() >= 90) {
    ultimaTela = agora;
    Serial.print("B10K="); Serial.print(pot);
    Serial.print(" POS="); Serial.print(pos);
    Serial.print(" ACS="); Serial.print(adcACS);
    Serial.print(" PWM="); Serial.print(pwmAtual);
    Serial.print(" ESTADO="); Serial.print((int)estado);
    Serial.print(" FALHA="); Serial.println(falha);
  }
}
