#include <Arduino.h>
#include <Servo.h>
#include <math.h>

// Arduino-Pico (Earle Philhower), RP2040. Numeros GP, nao pinos fisicos.
constexpr uint8_t POT_COMANDO = 26, POT_POSICAO = 27, ACS = 28;
constexpr uint8_t IN1 = 6, IN2 = 7, SERVO_NORMAL = 18;
constexpr int SERVO_MIN_US = 1000, SERVO_MAX_US = 2000;
constexpr int PWM_MAX = 64;  // 25% no primeiro teste do MG995 sem carga.
constexpr uint32_t PASSO_MS = 5;
constexpr float KP = 2.5f, KI = 0.0f, KD = 0.0f;
constexpr float ZONA_MORTA = 1.0f; // porcentagem do curso calibrado
// ACS desconhecido: limite em VOLTS NA SAIDA OUT, antes do divisor 10k/10k.
// 0.05 V equivale nominalmente a 0.27/0.50/0.76 A nas versoes 5/20/30 A.
// E um ponto inicial conservador; nao e um limite de corrente calibrado.
constexpr float LIMITE_DELTA_OUT_V = 0.05f;
constexpr float V_ADC = 3.3f, FATOR_DIVISOR = 2.0f;

Servo servo;
bool normalAtivo = false, inteligenteAtivo = false, verificando = false;
bool temA = false, temB = false, zeroValido = false, sentidoValido = false;
bool falha = false;
int extremoA = 0, extremoB = 0, polaridade = 1, pwmAplicado = 0;
int comandoRaw = 0, posicaoRaw = 0, correnteRaw = 0;
int inicioVerificacao = 0;
float zeroACS = 0, deltaOut = 0, alvoSuave = 50, integral = 0, erroAnterior = 0;
uint32_t ultimoPasso = 0, ultimaTela = 0, inicioTeste = 0, inicioSemMovimento = 0;
int referenciaMovimento = 0;
uint8_t excesso = 0;

int lerMedia(uint8_t pino) {
  int soma = 0;
  for (int i = 0; i < 8; ++i) soma += analogRead(pino);
  return soma / 8;
}

void motor(int pwm) {
  pwm = constrain(pwm, -PWM_MAX, PWM_MAX);
  // Primeiro desliga o lado oposto, inclusive ao inverter.
  if (pwm >= 0) {
    analogWrite(IN2, 0);
    analogWrite(IN1, pwm);
  } else {
    analogWrite(IN1, 0);
    analogWrite(IN2, -pwm);
  }
  pwmAplicado = pwm;
}

void parar() {
  inteligenteAtivo = false;
  verificando = false;
  normalAtivo = false;
  motor(0);
  if (servo.attached()) servo.detach();
  pinMode(SERVO_NORMAL, OUTPUT);
  digitalWrite(SERVO_NORMAL, LOW);
  integral = erroAnterior = 0;
  excesso = 0;
}

void bloquear(const char* motivo) {
  parar();
  falha = true;
  Serial.print("PARADA: "); Serial.println(motivo);
  Serial.println("Corrija a causa; x limpa a falha sem reiniciar movimento.");
}

bool cursoValido() {
  return temA && temB && abs(extremoB - extremoA) >= 300;
}

float posicaoPercentual() {
  return 100.0f * (posicaoRaw - extremoA) / (extremoB - extremoA);
}

bool requisitos(bool precisaSentido) {
  if (falha) { Serial.println("Falha travada. Corrija a causa e envie x."); return false; }
  if (!cursoValido()) { Serial.println("Calibre os dois extremos com a e b (curso minimo 300 ADC)."); return false; }
  if (!zeroValido) { Serial.println("Ponte alimentada e motor parado: envie z para zerar ACS."); return false; }
  if (precisaSentido && !sentidoValido) { Serial.println("Coloque o eixo no meio do curso e envie v."); return false; }
  return true;
}

void ajuda() {
  Serial.println("\ns: servo normal | i: inteligente | t: ambos | x: parar/limpar falha");
  Serial.println("a/b: registrar extremos manuais (motor parado) | z: zerar ACS");
  Serial.println("v: verificar sentido com pulso curto no meio do curso | ?: ajuda");
  Serial.println("Limites e zero sao temporarios: repetir apos reiniciar.");
}

void ativar(bool usarNormal, bool usarInteligente) {
  parar();
  posicaoRaw = lerMedia(POT_POSICAO);
  comandoRaw = lerMedia(POT_COMANDO);
  if (falha) { Serial.println("Corrija a falha e envie x."); return; }
  if (usarInteligente && !requisitos(true)) return;
  if (usarInteligente && (posicaoPercentual() < 0 || posicaoPercentual() > 100)) {
    Serial.println("Eixo fora do curso calibrado. Reposicione manualmente com motor parado."); return;
  }
  if (usarNormal) {
    int us = map(comandoRaw, 0, 4095, SERVO_MIN_US, SERVO_MAX_US);
    servo.attach(SERVO_NORMAL, SERVO_MIN_US, SERVO_MAX_US, us);
    if (!servo.attached()) { bloquear("Nao foi possivel gerar sinal do servo normal."); return; }
    normalAtivo = true;
  }
  if (usarInteligente) {
    alvoSuave = constrain(posicaoPercentual(), 3.0f, 97.0f);
    referenciaMovimento = posicaoRaw;
    inicioSemMovimento = millis();
    inteligenteAtivo = true;
  }
  Serial.println("Teste ativo. x interrompe os dois.");
}

void comando(char c) {
  if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  if (c == 'x') { parar(); falha = false; Serial.println("Parado."); }
  else if (c == '?') ajuda();
  else if (c == 's') ativar(true, false);
  else if (c == 'i') ativar(false, true);
  else if (c == 't') ativar(true, true);
  else if (c == 'a' || c == 'b') {
    parar();
    posicaoRaw = lerMedia(POT_POSICAO);
    if (c == 'a') { extremoA = posicaoRaw; temA = true; }
    else { extremoB = posicaoRaw; temB = true; }
    sentidoValido = false;
    Serial.print("Extremo "); Serial.print(c); Serial.print(" = "); Serial.println(posicaoRaw);
  } else if (c == 'z') {
    parar();
    // Motor desenergizado durante a coleta; nenhum movimento e iniciado aqui.
    delay(100);
    uint32_t soma = 0;
    for (int n = 0; n < 128; ++n) { soma += analogRead(ACS); delay(1); }
    zeroACS = soma / 128.0f;
    float v = zeroACS * V_ADC / 4095;
    zeroValido = (v >= 0.95f && v <= 1.55f);
    Serial.print("Zero ACS no GP28 = "); Serial.print(v, 3); Serial.println(" V");
    Serial.println(zeroValido ? "Zero aceito. Confirme que o ACS esta alimentado."
                            : "Zero rejeitado: confira 5V, GND e divisor 10k/10k.");
  } else if (c == 'v') {
    parar();
    posicaoRaw = lerMedia(POT_POSICAO);
    if (!requisitos(false)) return;
    if (posicaoPercentual() < 30 || posicaoPercentual() > 70) {
      Serial.println("Posicione manualmente o eixo no meio do curso (30 a 70%)."); return;
    }
    sentidoValido = false;
    inicioVerificacao = posicaoRaw;
    inicioTeste = millis();
    verificando = true;
    Serial.println("Pulso de verificacao: ate 150 ms, PWM limitado.");
  }
}

void atualizar() {
  comandoRaw = lerMedia(POT_COMANDO);
  posicaoRaw = lerMedia(POT_POSICAO);
  correnteRaw = lerMedia(ACS);
  deltaOut = fabsf(correnteRaw - zeroACS) * V_ADC / 4095 * FATOR_DIVISOR;

  if (normalAtivo) servo.writeMicroseconds(map(comandoRaw, 0, 4095, SERVO_MIN_US, SERVO_MAX_US));
  if (!inteligenteAtivo && !verificando) return;
  // Protecao complementar por software; nao substitui limitacao da fonte.
  excesso = deltaOut > LIMITE_DELTA_OUT_V ? excesso + 1 : 0;
  if (excesso >= 3) { bloquear("Variacao ACS acima do limite inicial."); return; }
  float pos = posicaoPercentual();
  if (pos < -2 || pos > 102 || posicaoRaw < 10 || posicaoRaw > 4085) {
    bloquear("Retorno de posicao fora da faixa."); return;
  }
  if (verificando) {
    int delta = posicaoRaw - inicioVerificacao;
    // Encerra ao detectar movimento suficiente ou ao esgotar o tempo.
    if (abs(delta) >= 12 || millis() - inicioTeste >= 150) {
      motor(0); verificando = false;
      if (abs(delta) < 8) { Serial.println("Sentido inconclusivo: confira motor/ponte; nao aumente PWM sem revisar."); return; }
      polaridade = ((delta > 0) == (extremoB > extremoA)) ? 1 : -1;
      sentidoValido = true;
      Serial.println("Sentido verificado. i = inteligente, t = ambos.");
    } else motor(PWM_MAX);
    return;
  }

  const float alvo = 3 + comandoRaw * (94.0f / 4095);
  alvoSuave += constrain(alvo - alvoSuave, -0.3f, 0.3f);
  const float erro = alvoSuave - pos;
  if (fabsf(erro) <= ZONA_MORTA) {
    motor(0); integral = 0; erroAnterior = erro;
    referenciaMovimento = posicaoRaw; inicioSemMovimento = millis(); return;
  }
  // PID com integracao condicional (anti-windup). Comeca somente com P.
  float candidato = integral + erro * 0.005f;
  float derivada = (erro - erroAnterior) / 0.005f;
  float saida = KP * erro + KI * candidato + KD * derivada;
  if (fabsf(saida) < PWM_MAX) integral = candidato;
  erroAnterior = erro;
  int pwm = (int)lroundf(constrain(saida, -(float)PWM_MAX, (float)PWM_MAX));
  if ((pos <= 0 && pwm < 0) || (pos >= 100 && pwm > 0)) pwm = 0;
  motor(polaridade * pwm);
  // Detecta falta de progresso; nao considera pequenas variacoes como movimento.
  if (abs(posicaoRaw - referenciaMovimento) >= 8 || abs(pwm) < 20) {
    referenciaMovimento = posicaoRaw; inicioSemMovimento = millis();
  } else if (millis() - inicioSemMovimento > 500) bloquear("Motor sem progresso de posicao.");
}

void setup() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  analogWriteRange(255); analogWriteFreq(20000);
  pinMode(SERVO_NORMAL, OUTPUT); digitalWrite(SERVO_NORMAL, LOW);
  analogReadResolution(12);
  pinMode(POT_COMANDO, INPUT); pinMode(POT_POSICAO, INPUT); pinMode(ACS, INPUT);
  Serial.begin(115200);
  motor(0);
}

void loop() {
  if (!Serial && (normalAtivo || inteligenteAtivo || verificando)) parar();
  // Limita comandos por volta para manter a leitura das protecoes sob trafego.
  for (int n = 0; n < 32 && Serial.available(); ++n) comando(Serial.read());
  uint32_t agora = millis();
  if (agora - ultimoPasso >= PASSO_MS) { ultimoPasso = agora; atualizar(); }
  // Evita bloquear a malha quando o computador nao esta lendo o Serial.
  if (Serial && agora - ultimaTela >= 500 && Serial.availableForWrite() >= 100) {
    ultimaTela = agora;
    Serial.print("B10K="); Serial.print(comandoRaw);
    Serial.print(" POS="); Serial.print(posicaoRaw);
    Serial.print(" ACS="); Serial.print(correnteRaw);
    Serial.print(" dOUT=");
    if (zeroValido) Serial.print(deltaOut, 3); else Serial.print("?");
    Serial.print("V PWM="); Serial.print(pwmAplicado);
    Serial.print(" N="); Serial.print(normalAtivo);
    Serial.print(" I="); Serial.print(inteligenteAtivo);
    Serial.print(" F="); Serial.println(falha);
  }
}
