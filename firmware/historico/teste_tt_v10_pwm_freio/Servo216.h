#pragma once
#include <stdint.h>
#include <math.h>

namespace Servo216 {
constexpr float CURSO_POT=216.0f, A=18.0f, B=198.0f, CORTE_A=9.0f, CORTE_B=207.0f;
constexpr float TOLERANCIA=1.2f;
constexpr float VELOCIDADE_MAX=20.0f, ACELERACAO_MAX=40.0f, JERK_MAX=160.0f;
constexpr int PWM_DETECCAO=200, PWM_MAX=200, PWM_PERTO_PONTA=200;
constexpr float KP=9.0f, KI=6.0f, KD=1.8f;
constexpr float INTEGRAL_MAX=180.0f, ANTI_WINDUP=2.0f;
constexpr float ATRITO_PWM=70.0f, FF_VELOCIDADE=0.6f;
constexpr float SUBIDA_PWM_POR_S=900.0f, DESCIDA_PWM_POR_S=3000.0f;
constexpr float graus(int raw) { return raw*(CURSO_POT/4095.0f); }
constexpr int adc(float g) { return (int)(g*(4095.0f/CURSO_POT)+0.5f); }
constexpr int sinal(float v) { return (v>0)-(v<0); }
constexpr float absoluto(float v) { return v<0 ? -v:v; }
constexpr float limitar(float v,float baixo,float alto) { return v<baixo ? baixo:(v>alto ? alto:v); }
constexpr bool seguro(float g) { return g>CORTE_A && g<CORTE_B; }
constexpr bool partidaValida(float g) { return g>=A+1 && g<=B-1; }
constexpr bool podeIniciar(float g,bool sentidoConhecido) { return sentidoConhecido ? seguro(g):partidaValida(g); }
constexpr float destinoInicial(float g) { return g>108 ? A:B; }
constexpr int pulsoServo(float alvo) { return 1000+(int)(limitar((alvo-A)/(B-A),0,1)*1000+0.5f); }
constexpr int limitarSaida(float g,int pwm) {
  return ((g<=A && pwm<0)||(g>=B && pwm>0)) ? 0:pwm;
}

// Perfil quintico: posicao, velocidade e aceleracao continuam suaves nas pontas.
// A duracao e escolhida para respeitar limites de velocidade, aceleracao e jerk.
class Trajetoria {
 public:
  float posicao=108, velocidade=0, aceleracao=0, duracao=1;
  bool concluida=true;
  void planejar(float inicio,float fim,uint32_t agora) {
    p0=inicio; p1=fim; inicioMs=agora;
    float distancia=absoluto(fim-inicio);
    duracao=fmaxf(0.5f,fmaxf(1.875f*distancia/VELOCIDADE_MAX,
             fmaxf(sqrtf(5.773503f*distancia/ACELERACAO_MAX),cbrtf(60*distancia/JERK_MAX))));
    posicao=inicio; velocidade=aceleracao=0; concluida=false;
  }
  void atualizar(uint32_t agora) {
    float u=limitar((agora-inicioMs)*0.001f/duracao,0,1);
    if(u>=1) { posicao=p1; velocidade=aceleracao=0; concluida=true; return; }
    float u2=u*u,u3=u2*u,u4=u3*u,u5=u4*u,distancia=p1-p0;
    posicao=p0+distancia*(10*u3-15*u4+6*u5);
    velocidade=distancia*(30*u2-60*u3+30*u4)/duracao;
    aceleracao=distancia*(60*u-180*u2+120*u3)/(duracao*duracao);
  }
 private:
  float p0=108,p1=108;
  uint32_t inicioMs=0;
};

// PID de posicao: integral para carga persistente e derivada filtrada pela
// velocidade. A parcela de atrito e continua; nao existem os chutes da V8.
class Malha {
 public:
  float angulo=108,velocidade=0,integral=0,saida=0;
  void iniciar(float g,uint32_t agora) {
    (void)agora; angulo=anterior=g; velocidade=integral=saida=0; dt=0.005f;
  }
  void observar(float g,float passo) {
    dt=limitar(passo,0.001f,0.05f);
    float instantanea=(g-anterior)/dt;
    velocidade+=(dt/(0.020f+dt))*(instantanea-velocidade);
    angulo=anterior=g;
  }
  int calcular(float alvo,float velAlvo,uint32_t agora) {
    (void)agora;
    float erro=alvo-angulo;
    float candidato=limitar(integral+(absoluto(erro)>0.15f ? KI*erro*dt:0),-INTEGRAL_MAX,INTEGRAL_MAX);
    float ff=ATRITO_PWM*tanhf((velAlvo+0.7f*erro)/4.0f)+FF_VELOCIDADE*velAlvo;
    float livre=KP*erro+candidato+KD*(velAlvo-velocidade)+ff;
    float distanciaPonta=fminf(angulo-A,B-angulo);
    int limite=distanciaPonta<10 ? PWM_PERTO_PONTA:PWM_MAX;
    float limitada=limitar(livre,-limite,limite);
    if((angulo<=A && limitada<0)||(angulo>=B && limitada>0)) limitada=0;
    // Realimentacao da saturacao evita acumulo de integral contra limites.
    integral=limitar(candidato+ANTI_WINDUP*(limitada-livre)*dt,-INTEGRAL_MAX,INTEGRAL_MAX);
    bool aumentando=(sinal(limitada)==sinal(saida) || absoluto(saida)<0.5f) && absoluto(limitada)>absoluto(saida);
    float taxa=aumentando ? SUBIDA_PWM_POR_S:DESCIDA_PWM_POR_S;
    saida+=limitar(limitada-saida,-taxa*dt,taxa*dt);
    int pwm=limitarSaida(angulo,(int)lroundf(saida));
    if(pwm==0 && ((angulo<=A && saida<0)||(angulo>=B && saida>0))) saida=0;
    return pwm;
  }
 private:
  float anterior=108,dt=0.005f;
};

// A aproximacao suave pode pedir PWM baixo durante mais de um segundo.
// So acusa travamento rapido quando a ponte esta efetivamente com forca alta.
class Progresso {
 public:
  void iniciar(int raw,uint32_t agora) { referencia=raw; ultimaMudanca=agora; forteMs=0; }
  bool travado(int raw,float erro,int pwm,float dt,uint32_t agora) {
    if(absoluto(raw-referencia)>=4 || absoluto(erro)<0.7f) { iniciar(raw,agora); return false; }
    if(absoluto(pwm)>=190) forteMs+=(uint32_t)(dt*1000+0.5f);
    else forteMs=0;
    return forteMs>1000 || (agora-ultimaMudanca>8000 && absoluto(erro)>2.5f);
  }
 private:
  int referencia=0;
  uint32_t ultimaMudanca=0,forteMs=0;
};
}
