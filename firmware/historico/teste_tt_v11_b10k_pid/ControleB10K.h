#pragma once
#include "Servo216.h"
namespace ControleB10K {
constexpr float VMAX=45.0f, AMAX=90.0f, JMAX=360.0f;
constexpr float destino(int raw) { return Servo216::A+Servo216::limitar(raw,0,4095)*(Servo216::B-Servo216::A)/4095.0f; }
constexpr float rpm(float grausPorSegundo) { return Servo216::absoluto(grausPorSegundo)/6.0f; }

// Seguidor online: preserva velocidade/aceleracao quando o usuario muda o alvo.
// Nao reinicia uma trajetoria em velocidade zero a cada leitura do potenciometro.
class Seguidor {
 public:
  float posicao=108,velocidade=0,aceleracao=0,pedido=108;
  void iniciar(float atual) {
    posicao=Servo216::limitar(atual,Servo216::A,Servo216::B);
    pedido=posicao; velocidade=aceleracao=0;
  }
  void atualizar(int raw,float dt,bool rapido) {
    using namespace Servo216;
    dt=limitar(dt,0.001f,0.05f);
    float novo=destino(raw);
    pedido+=(dt/((rapido?0.015f:0.040f)+dt))*(novo-pedido);
    if(rapido) { posicao=limitar(pedido,A,B); velocidade=aceleracao=0; return; }
    float erro=pedido-posicao;
    float accDesejada=limitar(9.0f*erro-6.0f*velocidade,-AMAX,AMAX);
    aceleracao+=limitar(accDesejada-aceleracao,-JMAX*dt,JMAX*dt);
    velocidade=limitar(velocidade+aceleracao*dt,-VMAX,VMAX);
    float proxima=posicao+velocidade*dt;
    // Corte de referencia tem prioridade caso uma reversao proxima a ponta
    // exija mais distancia de frenagem do que a faixa disponivel.
    if(proxima<A || proxima>B) {
      posicao=limitar(proxima,A,B); velocidade=aceleracao=0;
    } else posicao=proxima;
  }
};
}
