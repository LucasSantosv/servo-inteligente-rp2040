#include "../firmware/atual/teste_tt_v15_bancada_carga/TemperaturaNTC.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <initializer_list>
int main() {
 using namespace TemperaturaNTC;
 for(float t : {-20.0f,0.0f,25.0f,50.0f,75.0f,100.0f,125.0f}) {
  float r=100000.0f*std::exp(3950.0f*(1/(t+273.15f)-1/298.15f));
  int raw=(int)std::lround(4095.0f*r/(r+4700.0f)); float lida=0;
  assert(converter(raw,lida)==Ok && std::fabs(lida-t)<1.2f);
 }
 float t=123; assert(converter(4095,t)==Aberto && t==0); assert(converter(0,t)==Curto && t==0);
 Medidor m; assert(!m.valido()); m.atualizar(3911,.1f); assert(m.valido() && std::fabs(m.graus-25)<.5f);
 float fria=m.graus; m.atualizar(2500,.1f); assert(m.graus>fria && m.graus<100);
 m.atualizar(4095,.1f); assert(!m.valido() && m.graus==0); m.atualizar(3911,.1f); assert(m.valido() && std::fabs(m.graus-25)<.5f);
 std::puts("NTC: curva, filtro, abertura, curto e recuperacao OK.");
}

