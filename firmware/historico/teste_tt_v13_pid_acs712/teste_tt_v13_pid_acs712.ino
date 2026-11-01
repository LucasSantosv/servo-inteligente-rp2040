#include <Arduino.h>
#include <stdio.h>
#include <Servo.h>
#include "Servo216.h"
#include "PonteMX1508.h"
#include "ControleB10K.h"
#include "CorrenteACS712.h"
#include <hardware/pwm.h>
#include <hardware/clocks.h>

constexpr uint8_t B10K=26, POSICAO=27, ACS=28, IN1=6, IN2=7, SINAL_SERVO=18;
enum class Estado { Parado, Detectando, Acomodando, Oscilando, Pausa };
enum class Modo { Nenhum, Automatico, PotSuave, PotRapido };
Estado estado=Estado::Parado;
Modo modo=Modo::Nenhum;
Servo servo;
Servo216::Malha malha;
Servo216::Trajetoria trajetoria;
Servo216::Progresso progresso;
ControleB10K::Seguidor seguidor;
CorrenteACS712::Medidor corrente;
bool normalAtivo=false, sincronizado=false, falha=false;
bool plotter=true;
const char* motivoFalha="";
int pos=0, pot=0, inicioDeteccao=0, polaridade=0, pwmAtual=0;
float angulo=108, alvo=108, destino=Servo216::B;
float picoVelocidade=0;
float ultimoComandoProgresso=108;
uint32_t inicioEstado=0, ultimaLeitura=0, ultimoServo=0, ultimaTela=0, ultimoCorte=0;

int media(uint8_t pino) {
  int soma=0;
  for(int n=0;n<8;++n) soma+=analogRead(pino);
  return soma/8;
}

void escreverPonte(int pwm) {
  // GP6/GP7 sao A/B do mesmo slice; atualizacao conjunta evita estado intermediario.
  auto n=PonteMX1508::niveis(pwm,estado!=Estado::Parado);
  pwm_set_both_levels(pwm_gpio_to_slice_num(IN1),n.a,n.b);
  pwmAtual=pwm;
}

void motor(int pwm) {
  int teto=modo==Modo::PotRapido ? 255:Servo216::PWM_DETECCAO;
  pwm=constrain(pwm,-teto,teto);
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
  estado=Estado::Parado; escreverPonte(0); ultimoCorte=millis();
  modo=Modo::Nenhum;
  malha.iniciar(angulo,millis());
  normalAtivo=sincronizado=false;
  if(servo.attached()) servo.detach();
  pinMode(SINAL_SERVO,OUTPUT); digitalWrite(SINAL_SERVO,LOW);
}

void avisar(const char* texto) {
  if(!plotter) Serial.println(texto);
}

void bloquear(const char* motivo) {
  parar(); falha=true;
  motivoFalha=motivo;
  if(!plotter) { Serial.print("PAROU: "); Serial.println(motivo); }
  avisar("Falha registrada. x reconhece; selecione novamente p/f/i. Nao reinicia sozinho.");
}

bool ligarNormal(int us) {
  servo.attach(SINAL_SERVO,1000,2000,us);
  normalAtivo=servo.attached();
  if(!normalAtivo) bloquear("Falha no sinal do servo normal.");
  return normalAtivo;
}

bool modoPotenciometro() { return modo==Modo::PotSuave || modo==Modo::PotRapido; }

void prepararControle(uint32_t agora) {
  malha.iniciar(angulo,agora); alvo=angulo;
  if(modoPotenciometro()) seguidor.iniciar(angulo);
  else trajetoria.planejar(angulo,destino,agora);
  progresso.iniciar(pos,agora); estado=Estado::Oscilando;
  ultimoComandoProgresso=ControleB10K::destino(pot);
}

// Funcao de seguimento do B10K, chamada na mesma malha de 5 ms do PID.
void seguirPotenciometro(uint32_t agora,float dt) {
  bool rapido=modo==Modo::PotRapido;
  float pedido=ControleB10K::destino(pot);
  if(Servo216::absoluto(pedido-ultimoComandoProgresso)>1.0f) {
    progresso.registrarComando(agora); ultimoComandoProgresso=pedido;
  }
  seguidor.atualizar(pot,dt,rapido);
  alvo=seguidor.posicao;
  int pwm=malha.calcular(alvo,seguidor.velocidade,agora,rapido?255:Servo216::PWM_MAX,rapido);
  motor(pwm*polaridade);
  if(progresso.travado(pos,alvo-angulo,pwmAtual,dt,agora)) bloquear("Sem progresso no modo B10K.");
}

void mostrarVelocidade() {
  Serial.print("VEL="); Serial.print(malha.velocidade,1);
  Serial.print(" graus/s RPM="); Serial.print(ControleB10K::rpm(malha.velocidade),1);
  Serial.print(" PICO="); Serial.print(picoVelocidade,1);
  Serial.print(" graus/s PICO_RPM="); Serial.println(ControleB10K::rpm(picoVelocidade),1);
}

void iniciar(bool ambos,Modo novoModo) {
  corrente.cancelarZero();
  bool sentidoConhecido=polaridade!=0;
  parar();
  if(falha) { avisar("Confira a falha e envie x."); return; }
  pos=media(POSICAO); angulo=Servo216::graus(pos);
  if(!Servo216::podeIniciar(angulo,sentidoConhecido)) {
    avisar("Eixo perto da ponta. TT desligado: leve para perto do meio e envie p/f/i."); return;
  }
  modo=novoModo; picoVelocidade=0;
  alvo=angulo; destino=Servo216::destinoInicial(angulo);
  if(ambos && !ligarNormal(Servo216::pulsoServo(alvo))) return;
  sincronizado=ambos;
  inicioDeteccao=pos;
  inicioEstado=millis(); ultimaLeitura=inicioEstado; malha.iniciar(angulo,inicioEstado);
  if(sentidoConhecido) {
    prepararControle(inicioEstado);
  } else {
    estado=Estado::Detectando;
  }
  avisar(modo==Modo::PotSuave ? "V13 p: B10K suave com PID e ACS712."
               : modo==Modo::PotRapido ? "V13 f: B10K rapido, PID e ACS712, PWM ate 100%. x para parar."
               : "V13 i/t: vaivem automatico com PID e ACS712.");
}

void comando(char c) {
  if(c>='A' && c<='Z') c+='a'-'A';
  if(c=='x' || c=='r') {
    parar(); corrente.cancelarZero(); falha=false; motivoFalha="";
    if(c=='r') polaridade=0;
    avisar(c=='r' ? "Parado. Sentido apagado; novo modo faz deteccao.":"Parado. Selecione p/f/i para retomar.");
  }
  else if(c=='i') iniciar(false,Modo::Automatico);
  else if(c=='t') iniciar(true,Modo::Automatico);
  else if(c=='p') iniciar(false,Modo::PotSuave);
  else if(c=='f') iniciar(false,Modo::PotRapido);
  else if(c=='g') plotter=true;
  else if(c=='z') {
    parar(); corrente.iniciarZero(millis());
    avisar("Motor parado. Aguarde cerca de 1,2 s para zerar ACS712, sem corrente no sensor.");
  }
  else if(c>='0' && c<='3') {
    corrente.selecionar(c-'0');
    avisar(c=='1'?"ACS712 5 A (185 mV/A).":c=='2'?"ACS712 20 A (100 mV/A).":c=='3'?"ACS712 30 A (66 mV/A).":"ACS desconhecido: somente delta em mV.");
  }
  else if(c=='l' || c=='v') {
    plotter=false;
    if(falha) { Serial.print("PAROU: "); Serial.println(motivoFalha); }
    if(c=='v') mostrarVelocidade();
  }
  else if(c=='s') {
    parar(); corrente.cancelarZero();
    if(falha) { avisar("Confira a falha e envie x."); return; }
    pot=media(B10K);
    if(ligarNormal(map(pot,0,4095,1000,2000))) avisar("Servo normal pelo B10K.");
  } else if(c=='?') {
    plotter=false;
    Serial.println("g: Plotter PID | l: Monitor com mensagens | v: Monitor e velocidade");
    Serial.println("z: parar e zerar ACS | 1: ACS 5 A | 2: 20 A | 3: 30 A | 0: desconhecido");
    if(falha) { Serial.print("PAROU: "); Serial.println(motivoFalha); }
    Serial.println("p: TT pelo B10K suave | f: TT pelo B10K rapido 100% | v: velocidade e pico");
    Serial.println("i: vaivem | t: ambos vaivem | s: normal pelo B10K | x: parar");
    Serial.println("Sem a/b. Escala nominal: 0-4095 ADC = 0-216 graus, conforme curso informado.");
  }
}

void atualizar(uint32_t agora,float dt) {
  pot=media(B10K); pos=media(POSICAO); angulo=Servo216::graus(pos);
  int resultadoZero=corrente.atualizar(media(ACS),dt,agora);
  if(resultadoZero==1) avisar("Zero ACS aceito. Selecione 1/2/3 conforme a inscricao do sensor; p/f para mover.");
  else if(resultadoZero==-1) avisar("Zero ACS rejeitado: confira 5 V, GND, divisor e ausencia de corrente. Envie z novamente.");
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
      avisar("Sentido detectado. Preparando PID e perfil de aceleracao.");
    } else if(agora-inicioEstado>=1000) bloquear("Sem movimento na deteccao de sentido.");
    else motor((agora-inicioEstado)%80<20 ? Servo216::PWM_DETECCAO : 0);
    return;
  }
  if(estado==Estado::Acomodando) {
    motor(0);
    if(agora-inicioEstado>=200) {
      prepararControle(agora);
    }
    return;
  }
  if(estado==Estado::Oscilando || estado==Estado::Pausa)
    picoVelocidade=fmaxf(picoVelocidade,Servo216::absoluto(malha.velocidade));
  if(modoPotenciometro()) { seguirPotenciometro(agora,dt); return; }
  if(estado==Estado::Pausa) {
    // Continua segurando a posicao durante a pausa, em vez de soltar o eixo.
    motor(malha.calcular(alvo,0,agora)*polaridade);
    if(agora-inicioEstado>=300) {
      destino=destino>108 ? Servo216::A : Servo216::B;
      trajetoria.planejar(alvo,destino,agora);
      estado=Estado::Oscilando;
      progresso.iniciar(pos,agora);
    }
    return;
  }

  trajetoria.atualizar(agora); alvo=trajetoria.posicao;
  float velAlvo=trajetoria.velocidade;
  if(trajetoria.concluida && Servo216::absoluto(angulo-destino)<=Servo216::TOLERANCIA && Servo216::absoluto(malha.velocidade)<8) {
    estado=Estado::Pausa; inicioEstado=agora;
    motor(malha.calcular(alvo,0,agora)*polaridade);
    return;
  }
  int pwm=malha.calcular(alvo,velAlvo,agora);
  motor(pwm*polaridade);
  if(progresso.travado(pos,alvo-angulo,pwmAtual,dt,agora)) bloquear("Sem progresso de posicao.");
}

void setup() {
  digitalWrite(IN1,LOW); digitalWrite(IN2,LOW);
  pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT);
  pwm_config cfg=pwm_get_default_config();
  pwm_config_set_wrap(&cfg,255);
  pwm_config_set_clkdiv(&cfg,clock_get_hz(clk_sys)/(20000.0f*256.0f));
  uint slice=pwm_gpio_to_slice_num(IN1);
  pwm_init(slice,&cfg,false); pwm_set_both_levels(slice,0,0);
  gpio_set_function(IN1,GPIO_FUNC_PWM); gpio_set_function(IN2,GPIO_FUNC_PWM);
  pwm_set_enabled(slice,true);
  pinMode(SINAL_SERVO,OUTPUT); digitalWrite(SINAL_SERVO,LOW);
  analogReadResolution(12);
  pinMode(B10K,INPUT); pinMode(POSICAO,INPUT); pinMode(ACS,INPUT);
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
  if(Serial && plotter && agora-ultimaTela>=50) {
    ultimaTela=agora;
    char linha[320];
    int n=snprintf(linha,sizeof(linha),
      "Alvo:%.1f\tPosicao:%.1f\tP:%.1f\tI:%.1f\tD:%.1f\tFF:%.1f\tPWM:%d\tFalha:%d\tACS_mV:%.1f\tCorrente_mA:%.1f\tACS_OK:%d\tAmp_OK:%d\r\n",
      alvo,angulo,malha.parcelaP,malha.parcelaI,malha.parcelaD,malha.parcelaFF,pwmAtual,(int)falha,
      corrente.deltaMV(),corrente.miliamperes(),(int)corrente.zeroValido(),(int)corrente.amperesValidos());
    // Descarta o quadro se nao couber: o grafico nao deve bloquear o PID.
    if(n>0 && n<(int)sizeof(linha) && Serial.availableForWrite()>=n)
      Serial.write((const uint8_t*)linha,n);
  }
  if(Serial && !plotter && agora-ultimaTela>=100) {
    ultimaTela=agora;
    char linha[320];
    int n=snprintf(linha,sizeof(linha),
      "V13 POS=%d POT=%.1f ANG=%.1f ALVO=%.1f VEL=%.1f RPM=%.1f PICO_RPM=%.1f PWM=%d ESTADO=%d FALHA=%d ACS_ADC=%d ACS_mV=%.1f mA=%.1f ACS_OK=%d Amp_OK=%d ESCALA=%d ZERO=%d\r\n",
      pos,ControleB10K::destino(pot),angulo,alvo,malha.velocidade,ControleB10K::rpm(malha.velocidade),
      ControleB10K::rpm(picoVelocidade),pwmAtual,(int)estado,(int)falha,corrente.raw,corrente.deltaMV(),
      corrente.miliamperes(),(int)corrente.zeroValido(),(int)corrente.amperesValidos(),corrente.variante,(int)corrente.zerando);
    if(n>0 && n<(int)sizeof(linha) && Serial.availableForWrite()>=n)
      Serial.write((const uint8_t*)linha,n);
  }
}
