#pragma once
#include "Servo216.h"

// Variante de bancada: ausencia de progresso e diagnostico, nao falha de movimento.
// Nao interfere no calculo do PID nem no comando de potencia.
class AvisoProgresso {
 public:
  bool ativo=false;
  bool atualizar(Servo216::Progresso& medidor,int raw,float erro,int pwm,float dt,uint32_t agora) {
    bool anterior=ativo;
    ativo=medidor.travado(raw,erro,pwm,dt,agora);
    return ativo!=anterior;
  }
  void limpar() { ativo=false; }
};
