#include <Arduino.h>
#include <Servo.h>
#include <math.h>
#include "Curso180.h"

constexpr uint8_t B10K = 26, POSICAO = 27, IN1 = 6, IN2 = 7, SINAL_SERVO = 18;
enum class Estado { Parado, Detectando, Oscilando, Pausa };
Estado estado = Estado::Parado;
Servo servo;
bool temA = false, temB = false, falha = false, normalAtivo = false, sincronizado = false;
int pontoA = 0, pontoB = 0, pos = 0, pot = 0, inicioDeteccao = 0, polaridade = 0;
int referenciaSentido = 0, sentidoEsperado = 0, referenciaProgresso = 0, pwmAtual = 0;
float alvo = 90, destino = 180;
uint32_t inicioEstado = 0, ultimaLeitura = 0, ultimoServo = 0, ultimaTela = 0;
uint32_t inicioProgresso = 0, ultimaInversao = 0;

int media(uint8_t pino) {
  int soma = 0;
  for (int n = 0; n < 8; ++n) soma += analogRead(pino);
  return soma / 8;
}

bool calibrado() { return temA && temB && Curso180::valido(pontoA, pontoB); }

void motor(int pwm) {
  pwm = constrain(pwm, -Curso180::PWM, Curso180::PWM);
  if (pwm >= 0) { analogWrite(IN2, 0); analogWrite(IN1, pwm); }
  else { analogWrite(IN1, 0); analogWrite(IN2, -pwm); }
  pwmAtual = pwm;
}

void parar() {
  motor(0); estado = Estado::Parado;
  normalAtivo = sincronizado = false;
  sentidoEsperado = polaridade = 0;
  if (servo.attached()) servo.detach();
  pinMode(SINAL_SERVO, OUTPUT); digitalWrite(SINAL_SERVO, LOW);
}

void bloquear(const char* motivo) {
  parar(); falha = true;
  Serial.print("PAROU: "); Serial.println(motivo);
  Serial.println("Corrija a causa e envie x. Nao reinicia sozinho.");
}

bool ligarNormal(int us) {
  servo.attach(SINAL_SERVO, 1000, 2000, us);
  normalAtivo = servo.attached();
  if (!normalAtivo) bloquear("Falha no sinal do servo normal.");
  return normalAtivo;
}

void iniciarOscilacao(bool ambos) {
  parar();
  if (falha) { Serial.println("Confira a falha e envie x."); return; }
  if (!calibrado()) { Serial.println("Marque 180 graus fisicos e registre os pontos com a e b."); return; }
  pos = media(POSICAO);
  if (!Curso180::centro(pos, pontoA, pontoB)) {
    Serial.println("Com TT desligado, coloque o eixo no meio entre A e B. Depois i/t."); return;
  }
  alvo = Curso180::graus(pos, pontoA, pontoB);
  if (ambos && !ligarNormal(Curso180::pulsoServo(alvo))) return;
  sincronizado = ambos;
  inicioDeteccao = pos;
  inicioEstado = millis();
  estado = Estado::Detectando;
  motor(Curso180::PWM);
  Serial.println("V6 SEM ACS: girando e detectando sentido; depois vaivem continuo A-B.");
}

void comando(char c) {
  if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  if (c == 'x') { parar(); falha = false; Serial.println("Parado."); }
  else if (c == 'a' || c == 'b') {
    parar(); pos = media(POSICAO);
    if (c == 'a') { pontoA = pos; temA = true; }
    else { pontoB = pos; temB = true; }
    Serial.print("Ponto "); Serial.print(c); Serial.print(" = "); Serial.println(pos);
    if (calibrado()) Serial.println("Pontos registrados. Eixo no meio, depois i para vaivem.");
  } else if (c == 'i') iniciarOscilacao(false);
  else if (c == 't') iniciarOscilacao(true);
  else if (c == 's') {
    parar();
    if (falha) { Serial.println("Confira a falha e envie x."); return; }
    pot = media(B10K);
    if (ligarNormal(map(pot, 0, 4095, 1000, 2000))) Serial.println("Servo normal pelo B10K.");
  } else if (c == '?') {
    Serial.println("a/b: pontos fisicos separados por 180 graus (TT sem alimentacao).");
    Serial.println("i: TT vaivem continuo | t: ambos no vaivem | s: normal pelo B10K | x: parar");
  }
}

void atualizar(uint32_t agora) {
  pot = media(B10K); pos = media(POSICAO);
  if (normalAtivo && agora - ultimoServo >= 20) {
    ultimoServo = agora;
    servo.writeMicroseconds(sincronizado ? Curso180::pulsoServo(alvo) : map(pot, 0, 4095, 1000, 2000));
  }
  if (estado == Estado::Parado) return;
  if (!calibrado() || !Curso180::dentro(pos, pontoA, pontoB)) {
    bloquear("Retorno passou da faixa A-B ou leitura invalida."); return;
  }
  float angulo = Curso180::graus(pos, pontoA, pontoB);
  if (estado == Estado::Detectando) {
    int delta = pos - inicioDeteccao;
    if (abs(delta) >= 12) {
      // Identifica durante o movimento e transfere imediatamente ao controle.
      polaridade = Curso180::polaridade(delta, pontoA, pontoB);
      alvo = angulo;
      destino = 180;
      referenciaSentido = referenciaProgresso = pos;
      inicioProgresso = ultimaInversao = agora;
      sentidoEsperado = 0;
      estado = Estado::Oscilando;
      motor(0); // Proximo ciclo ja controla posicao; nao existe pausa de calibracao.
      Serial.println("Sentido detectado. Vaivem continuo entre 0 e 180 graus. x para parar.");
    } else if (agora - inicioEstado >= 1000) bloquear("Nao detectei movimento em um segundo.");
    return;
  }
  if (estado == Estado::Pausa) {
    motor(0);
    if (agora - inicioEstado >= 300) {
      destino = destino > 90 ? 0 : 180;
      estado = Estado::Oscilando;
      referenciaSentido = referenciaProgresso = pos;
      inicioProgresso = ultimaInversao = agora;
      sentidoEsperado = 0;
    }
    return;
  }
  if (agora - ultimaInversao >= 80 && Curso180::contrario(referenciaSentido, pos, pontoA, pontoB, sentidoEsperado)) {
    bloquear("Retorno indica sentido contrario ao comando."); return;
  }
  if ((pos - referenciaSentido) * Curso180::sinal(pontoB - pontoA) * sentidoEsperado > 0)
    referenciaSentido = pos;

  float passo = Curso180::VELOCIDADE_ALVO * 0.002f;
  alvo += constrain(destino - alvo, -passo, passo);
  float erro = alvo - angulo;
  if (fabsf(alvo - destino) < 0.01f && fabsf(angulo - destino) <= Curso180::TOLERANCIA) {
    motor(0); estado = Estado::Pausa; inicioEstado = agora;
    sentidoEsperado = 0;
    return;
  }
  if (fabsf(erro) <= Curso180::TOLERANCIA) {
    motor(0); sentidoEsperado = 0;
    referenciaSentido = referenciaProgresso = pos;
    inicioProgresso = agora;
    return;
  }
  int novoSentido = erro > 0 ? 1 : -1;
  if (novoSentido != sentidoEsperado) {
    sentidoEsperado = novoSentido;
    referenciaSentido = referenciaProgresso = pos;
    inicioProgresso = ultimaInversao = agora;
  }
  // O alvo percorre o arco gradualmente. O motor e acionado somente enquanto
  // houver erro de posicao, com a potencia que ja fez o TT partir nesta montagem.
  motor(sentidoEsperado * polaridade * Curso180::PWM);
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
  pinMode(B10K, INPUT); pinMode(POSICAO, INPUT);
  Serial.begin(115200); motor(0);
}

void loop() {
  if (!Serial && (estado != Estado::Parado || normalAtivo)) parar();
  for (int n = 0; n < 16 && Serial.available(); ++n) comando(Serial.read());
  uint32_t agora = millis();
  if (agora - ultimaLeitura >= 2) { ultimaLeitura = agora; atualizar(agora); }
  if (Serial && agora - ultimaTela >= 500 && Serial.availableForWrite() >= 110) {
    ultimaTela = agora;
    Serial.print("POS="); Serial.print(pos);
    Serial.print(" ANG=");
    if (calibrado()) Serial.print(Curso180::graus(pos, pontoA, pontoB), 1); else Serial.print("?");
    Serial.print(" ALVO="); Serial.print(alvo, 1);
    Serial.print(" PWM="); Serial.print(pwmAtual);
    Serial.print(" ESTADO="); Serial.print((int)estado);
    Serial.print(" FALHA="); Serial.println(falha);
  }
}
