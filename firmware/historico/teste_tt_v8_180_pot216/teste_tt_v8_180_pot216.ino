#include <Arduino.h>
#include <Servo.h>
#include "Servo216.h"

constexpr uint8_t B10K=26, POSICAO=27, IN1=6, IN2=7, SINAL_SERVO=18;
enum class Estado { Parado, Detectando, Acomodando, Oscilando, Pausa };
Estado estado=Estado::Parado;
Servo servo;
Servo216::Malha malha;
bool normalAtivo=false, sincronizado=false, falha=false;
int pos=0, pot=0, inicioDeteccao=0, polaridade=0, pwmAtual=0, referenciaProgresso=0;
float angulo=108, alvo=108, destino=Servo216::B;
uint32_t inicioEstado=0, ultimaLeitura=0, ultimoServo=0, ultimaTela=0, ultimoCorte=0;
uint32_t inicioProgresso=0;

int media(uint8_t pino) {
  int soma=0;
  for(int n=0;n<8;++n) soma+=analogRead(pino);
  return soma/8;
}

void escreverPonte(int pwm) {
  if(pwm>=0) { analogWrite(IN2,0); analogWrite(IN1,pwm); }
  else { analogWrite(IN1,0); analogWrite(IN2,-pwm); }
  pwmAtual=pwm;
}

void motor(int pwm) {
  pwm=constrain(pwm,-Servo216::PWM_DETECCAO,Servo216::PWM_DETECCAO);
  uint32_t agora=millis();
  // Intervalo desligado ao inverter o comando da ponte.
  if(pwmAtual!=0 && (pwm==0 || Servo216::sinal(pwm)!=Servo216::sinal(pwmAtual))) {
    escreverPonte(0); ultimoCorte=agora;
    return;
  }
  if(pwm!=0 && agora-ultimoCorte<5) return;
  escreverPonte(pwm);
}

void parar() {
  motor(0); estado=Estado::Parado;
  normalAtivo=sincronizado=false;
  if(servo.attached()) servo.detach();
  pinMode(SINAL_SERVO,OUTPUT); digitalWrite(SINAL_SERVO,LOW);
}

void bloquear(const char* motivo) {
  parar(); falha=true;
  Serial.print("PAROU: "); Serial.println(motivo);
  Serial.println("Confira a montagem; x para reconhecer. Nao reinicia sozinho.");
}

bool ligarNormal(int us) {
  servo.attach(SINAL_SERVO,1000,2000,us);
  normalAtivo=servo.attached();
  if(!normalAtivo) bloquear("Falha no sinal do servo normal.");
  return normalAtivo;
}

void iniciar(bool ambos) {
  parar();
  if(falha) { Serial.println("Confira a falha e envie x."); return; }
  pos=media(POSICAO); angulo=Servo216::graus(pos);
  if(!Servo216::partidaValida(angulo)) {
    Serial.println("Eixo perto da ponta. TT desligado: leve para perto do meio e envie i."); return;
  }
  alvo=angulo; destino=Servo216::B;
  if(ambos && !ligarNormal(Servo216::pulsoServo(alvo))) return;
  sincronizado=ambos;
  inicioDeteccao=pos; polaridade=0;
  inicioEstado=millis(); malha.iniciar(angulo,inicioEstado);
  estado=Estado::Detectando;
  Serial.println("V8 SEM ACS: pot 216 graus; vaivem 18 a 198 graus (180 de percurso).");
}

void comando(char c) {
  if(c>='A' && c<='Z') c+='a'-'A';
  if(c=='x' || c=='r') { parar(); falha=false; Serial.println("Parado."); }
  else if(c=='i') iniciar(false);
  else if(c=='t') iniciar(true);
  else if(c=='s') {
    parar();
    if(falha) { Serial.println("Confira a falha e envie x."); return; }
    pot=media(B10K);
    if(ligarNormal(map(pot,0,4095,1000,2000))) Serial.println("Servo normal pelo B10K.");
  } else if(c=='?') {
    Serial.println("i: TT vaivem 180 | t: ambos | s: normal pelo B10K | x: parar");
    Serial.println("Sem a/b. Escala nominal: 0-4095 ADC = 0-216 graus, conforme curso informado.");
  }
}

void atualizar(uint32_t agora,float dt) {
  pot=media(B10K); pos=media(POSICAO); angulo=Servo216::graus(pos);
  if(normalAtivo && agora-ultimoServo>=20) {
    ultimoServo=agora;
    servo.writeMicroseconds(sincronizado ? Servo216::pulsoServo(alvo) : map(pot,0,4095,1000,2000));
  }
  if(estado==Estado::Parado) return;
  if(dt>0.05f) { bloquear("Ciclo de controle atrasado."); return; }
  if(!Servo216::seguro(angulo)) { bloquear("Posicao saiu da margem de 9 a 207 graus."); return; }
  malha.observar(angulo,dt);

  if(estado==Estado::Detectando) {
    int delta=pos-inicioDeteccao;
    if(abs(delta)>=10) {
      motor(0); polaridade=Servo216::sinal(delta);
      estado=Estado::Acomodando; inicioEstado=agora;
      Serial.println("Sentido detectado. Preparando acompanhamento com desaceleracao.");
    } else if(agora-inicioEstado>=1000) bloquear("Sem movimento na deteccao de sentido.");
    else motor((agora-inicioEstado)%80<20 ? Servo216::PWM_DETECCAO : 0);
    return;
  }
  if(estado==Estado::Acomodando) {
    motor(0);
    if(agora-inicioEstado>=80) {
      malha.iniciar(angulo,agora); alvo=angulo;
      referenciaProgresso=pos; inicioProgresso=agora;
      estado=Estado::Oscilando;
    }
    return;
  }
  if(estado==Estado::Pausa) {
    motor(0);
    if(agora-inicioEstado>=300) {
      destino=destino>108 ? Servo216::A : Servo216::B;
      estado=Estado::Oscilando;
      referenciaProgresso=pos; inicioProgresso=agora;
    }
    return;
  }

  float passo=Servo216::VELOCIDADE_ALVO*dt;
  alvo+=Servo216::limitar(destino-alvo,-passo,passo);
  float velAlvo=Servo216::absoluto(alvo-destino)<0.001f ? 0 : Servo216::VELOCIDADE_ALVO*Servo216::sinal(destino-alvo);
  if(Servo216::absoluto(alvo-destino)<0.001f && Servo216::absoluto(angulo-destino)<=Servo216::TOLERANCIA && Servo216::absoluto(malha.velocidade)<8) {
    motor(0); estado=Estado::Pausa; inicioEstado=agora;
    return;
  }
  int pwm=malha.calcular(alvo,velAlvo,agora);
  motor(pwm*polaridade);
  if(abs(pos-referenciaProgresso)>=4 || Servo216::absoluto(alvo-angulo)<1.0f) {
    referenciaProgresso=pos; inicioProgresso=agora;
  } else if(agora-inicioProgresso>1000) bloquear("Sem progresso de posicao.");
}

void setup() {
  digitalWrite(IN1,LOW); digitalWrite(IN2,LOW);
  pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT);
  analogWriteRange(255); analogWriteFreq(20000);
  pinMode(SINAL_SERVO,OUTPUT); digitalWrite(SINAL_SERVO,LOW);
  analogReadResolution(12);
  pinMode(B10K,INPUT); pinMode(POSICAO,INPUT);
  Serial.begin(115200); escreverPonte(0);
}

void loop() {
  if(!Serial && (estado!=Estado::Parado || normalAtivo)) parar();
  for(int n=0;n<16 && Serial.available();++n) comando(Serial.read());
  uint32_t agora=millis();
  if(agora-ultimaLeitura>=5) {
    float dt=(agora-ultimaLeitura)*0.001f;
    ultimaLeitura=agora; atualizar(agora,dt);
  }
  if(Serial && agora-ultimaTela>=250 && Serial.availableForWrite()>=100) {
    ultimaTela=agora;
    Serial.print("V8 POS="); Serial.print(pos);
    Serial.print(" ANG="); Serial.print(angulo,1);
    Serial.print(" ALVO="); Serial.print(alvo,1);
    Serial.print(" VEL="); Serial.print(malha.velocidade,1);
    Serial.print(" PWM="); Serial.print(pwmAtual);
    Serial.print(" ESTADO="); Serial.print((int)estado);
    Serial.print(" FALHA="); Serial.println(falha);
  }
}
