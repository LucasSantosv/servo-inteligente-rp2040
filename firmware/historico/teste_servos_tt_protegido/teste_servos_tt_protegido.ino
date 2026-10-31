#include <Arduino.h>
#include <Servo.h>
#include <math.h>
#include "LimitesTT.h"

// Arduino-Pico (Earle Philhower), RP2040. Numeros GP, nao pinos fisicos.
constexpr uint8_t POT_COMANDO = 26, POT_POSICAO = 27, ACS = 28;
constexpr uint8_t IN1 = 6, IN2 = 7, SERVO_NORMAL = 18;
constexpr int SERVO_MIN_US = 1000, SERVO_MAX_US = 2000;
constexpr int PWM_MAX = 32;  // 12.5% para o primeiro teste do TT sem carga.
constexpr uint32_t PULSO_SENTIDO_MS = 40;
constexpr uint32_t PASSO_MS = 5;
constexpr float KP = 1.4f, KI = 0.0f, KD = 0.0f;
constexpr float ZONA_MORTA = 2.0f; // graus nominais do curso calibrado
// ACS desconhecido: limite em VOLTS NA SAIDA OUT, antes do divisor 10k/10k.
// 0.05 V equivale nominalmente a 0.27/0.50/0.76 A nas versoes 5/20/30 A.
// E um ponto inicial conservador; nao e um limite de corrente calibrado.
constexpr float LIMITE_DELTA_OUT_V = 0.05f;
constexpr float V_ADC = 3.3f, FATOR_DIVISOR = 2.0f;

Servo servo;
bool normalAtivo = false, inteligenteAtivo = false, verificando = false;
bool temA = false, temB = false, zeroValido = false, sentidoValido = false;
bool falha = false;
bool arcoConfirmado = false;
int extremoA = 0, extremoB = 0, polaridade = 1, pwmAplicado = 0;
int comandoRaw = 0, posicaoRaw = 0, correnteRaw = 0;
int inicioVerificacao = 0;
float zeroACS = 0, deltaOut = 0, alvoSuave = 50, integral = 0, erroAnterior = 0;
uint32_t ultimoPasso = 0, ultimaTela = 0, inicioTeste = 0, inicioSemMovimento = 0;
int referenciaMovimento = 0;
int referenciaSentido = 0, sentidoEsperado = 0, ultimaPosicaoRaw = 0;
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
  sentidoEsperado = 0;
}

void bloquear(const char* motivo) {
  parar();
  falha = true;
  sentidoValido = false;
  Serial.print("PARADA: "); Serial.println(motivo);
  Serial.println("Corrija a causa; x limpa a falha sem reiniciar movimento.");
}

bool cursoValido() {
  return temA && temB && LimitesTT::calibracaoValida(extremoA, extremoB);
}

float posicaoGraus() {
  return LimitesTT::graus(posicaoRaw, extremoA, extremoB);
}

bool requisitos(bool precisaSentido) {
  if (falha) { Serial.println("Falha travada. Corrija a causa e envie x."); return false; }
  if (!cursoValido()) { Serial.println("Calibre os dois extremos com a e b (curso minimo 300 ADC)."); return false; }
  if (!arcoConfirmado) { Serial.println("Meça o arco fisico A-B no eixo TT: no maximo 180 graus. Depois envie c."); return false; }
  if (!zeroValido) { Serial.println("Ponte alimentada e motor parado: envie z para zerar ACS."); return false; }
  if (precisaSentido && !sentidoValido) { Serial.println("Coloque o eixo no meio do curso e envie v."); return false; }
  return true;
}

void ajuda() {
  Serial.println("\ns: servo normal | i: inteligente | t: ambos | x: parar/limpar falha");
  Serial.println("a/b: registrar extremos manuais (motor parado) | z: zerar ACS");
  Serial.println("c: confirmar que o arco fisico A-B foi medido e e <=180 graus");
  Serial.println("v: verificar sentido com pulso curto no meio do curso | ?: ajuda");
  Serial.println("Limites e zero sao temporarios: repetir apos reiniciar.");
}

void ativar(bool usarNormal, bool usarInteligente) {
  parar();
  posicaoRaw = lerMedia(POT_POSICAO);
  comandoRaw = lerMedia(POT_COMANDO);
  if (falha) { Serial.println("Corrija a falha e envie x."); return; }
  if (usarInteligente && !requisitos(true)) return;
  if (usarInteligente && !LimitesTT::dentro(posicaoGraus())) {
    Serial.println("Eixo fora do curso calibrado. Reposicione manualmente com motor parado."); return;
  }
  if (usarNormal) {
    int us = map(comandoRaw, 0, 4095, SERVO_MIN_US, SERVO_MAX_US);
    servo.attach(SERVO_NORMAL, SERVO_MIN_US, SERVO_MAX_US, us);
    if (!servo.attached()) { bloquear("Nao foi possivel gerar sinal do servo normal."); return; }
    normalAtivo = true;
  }
  if (usarInteligente) {
    alvoSuave = constrain(posicaoGraus(), LimitesTT::ALVO_MIN, LimitesTT::ALVO_MAX);
    referenciaMovimento = posicaoRaw;
    referenciaSentido = ultimaPosicaoRaw = posicaoRaw;
    inicioSemMovimento = millis();
    inteligenteAtivo = true;
  }
  Serial.println("Teste ativo. x interrompe os dois.");
}

void comando(char c) {
  if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  if (c == 'x') { parar(); falha = false; sentidoValido = false; Serial.println("Parado. Para TT, repita v antes de i/t."); }
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
    arcoConfirmado = false;
    Serial.print("Extremo "); Serial.print(c); Serial.print(" = "); Serial.println(posicaoRaw);
  } else if (c == 'c') {
    parar();
    arcoConfirmado = cursoValido();
    sentidoValido = false;
    Serial.println(arcoConfirmado ? "Arco A-B declarado <=180 graus. Isso depende da sua medicao fisica."
                                : "Registre a/b validos antes de confirmar o arco.");
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
    if (!LimitesTT::centro(posicaoGraus())) {
      Serial.println("Posicione manualmente o eixo no centro (75 a 105 graus nominais)."); return;
    }
    sentidoValido = false;
    inicioVerificacao = posicaoRaw;
    ultimaPosicaoRaw = posicaoRaw;
    inicioTeste = millis();
    verificando = true;
    Serial.println("Pulso de sentido: ate 40 ms, PWM maximo 12.5%. Prepare x/corte de alimentacao.");
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
  float pos = posicaoGraus();
  if (!LimitesTT::dentro(pos) || posicaoRaw < 50 || posicaoRaw > 4045) {
    bloquear("Retorno de posicao fora da faixa."); return;
  }
  float salto = fabsf(LimitesTT::graus(posicaoRaw, extremoA, extremoB) -
                       LimitesTT::graus(ultimaPosicaoRaw, extremoA, extremoB));
  ultimaPosicaoRaw = posicaoRaw;
  if (salto > 8.0f) { bloquear("Retorno mudou mais de 8 graus nominais em uma leitura."); return; }
  if (verificando) {
    int delta = posicaoRaw - inicioVerificacao;
    // Encerra ao detectar movimento suficiente ou ao esgotar o tempo.
    if (abs(delta) >= 12 || millis() - inicioTeste >= PULSO_SENTIDO_MS) {
      motor(0); verificando = false;
      if (abs(delta) < 8) { Serial.println("Sentido inconclusivo: confira motor/ponte; nao aumente PWM sem revisar."); return; }
      polaridade = LimitesTT::polaridade(delta, extremoA, extremoB);
      sentidoValido = true;
      Serial.println("Sentido verificado. i = inteligente, t = ambos.");
    } else motor(PWM_MAX);
    return;
  }

  // Verifica o movimento observado desde o inicio da ultima direcao comandada.
  if (LimitesTT::contrario(referenciaSentido, posicaoRaw, extremoA, extremoB, sentidoEsperado)) {
    bloquear("Motor moveu no sentido contrario ao esperado."); return;
  }
  // Atualiza a referencia somente no sentido correto: detecta retorno adverso
  // de 8 ADC mesmo depois de um movimento longo no sentido correto.
  if (LimitesTT::deltaLogico(referenciaSentido, posicaoRaw, extremoA, extremoB) * sentidoEsperado > 0)
    referenciaSentido = posicaoRaw;
  const float alvo = LimitesTT::ALVO_MIN + comandoRaw *
                     ((LimitesTT::ALVO_MAX - LimitesTT::ALVO_MIN) / 4095);
  alvoSuave += constrain(alvo - alvoSuave, -0.15f, 0.15f); // ate 30 graus/s
  const float erro = alvoSuave - pos;
  if (fabsf(erro) <= ZONA_MORTA) {
    motor(0); integral = 0; erroAnterior = erro;
    sentidoEsperado = 0; referenciaSentido = posicaoRaw;
    referenciaMovimento = posicaoRaw; inicioSemMovimento = millis(); return;
  }
  // PID com integracao condicional (anti-windup). Comeca somente com P.
  float candidato = integral + erro * 0.005f;
  float derivada = (erro - erroAnterior) / 0.005f;
  float saida = KP * erro + KI * candidato + KD * derivada;
  if (fabsf(saida) < PWM_MAX) integral = candidato;
  erroAnterior = erro;
  int pwm = (int)lroundf(constrain(saida, -(float)PWM_MAX, (float)PWM_MAX));
  int novoSentido = LimitesTT::sinal(pwm);
  if (novoSentido != sentidoEsperado) {
    sentidoEsperado = novoSentido;
    referenciaSentido = posicaoRaw;
  }
  motor(polaridade * pwm);
  // Detecta falta de progresso; nao considera pequenas variacoes como movimento.
  if (abs(posicaoRaw - referenciaMovimento) >= 8 || pwm == 0) {
    referenciaMovimento = posicaoRaw; inicioSemMovimento = millis();
  } else if (millis() - inicioSemMovimento > 150) bloquear("Motor sem progresso de posicao.");
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
