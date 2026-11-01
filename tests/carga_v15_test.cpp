#include "../firmware/atual/teste_tt_v15_bancada_carga/AvisoProgresso.h"
#include <cassert>
#include <cstdio>
int main(){
 Servo216::Progresso p; AvisoProgresso aviso; p.iniciar(2048,0);
 Servo216::Malha controle; controle.iniciar(108,0);
 int pwm=0;
 for(unsigned ms=5;ms<=12000;ms+=5){
  controle.observar(108,.005f); pwm=controle.calcular(160,0,ms,255,true);
  aviso.atualizar(p,2048,52,pwm,.005f,ms);
 }
 assert(aviso.ativo && pwm>0);
 aviso.atualizar(p,2058,51,pwm,.005f,12005); assert(!aviso.ativo);
 p.iniciar(2048,0); aviso.limpar();
 for(unsigned ms=5;ms<=12000;ms+=5) aviso.atualizar(p,2048,20,100,.005f,ms);
 assert(aviso.ativo); aviso.limpar(); assert(!aviso.ativo);
 assert(!Servo216::seguro(8) && !Servo216::seguro(208));
 std::puts("Carga mantida: aviso sem zerar PID/PWM; recuperacao e limpeza OK; limites externos preservados.");
}

