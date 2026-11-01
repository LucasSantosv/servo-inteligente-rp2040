#pragma once
#include <math.h>

namespace TemperaturaNTC {
// Mesmo sensor configurado na extrusora: R25=100k, Beta=3950.
// Divisor: 3V3 -> 4k7 -> ADC -> NTC -> GND, alimentado pela referencia ADC.
constexpr float R_FIXO=4700.0f, R25=100000.0f, BETA=3950.0f, T25_K=298.15f;
enum Estado { Aguardando=0, Ok=1, Curto=2, Aberto=3, ForaFaixa=4 };
inline Estado converter(int raw,float& graus) {
  graus=0;
  if(raw<=4) return Curto;
  if(raw>=4091) return Aberto;
  float resistencia=R_FIXO*raw/(4095.0f-raw);
  float invT=1.0f/T25_K+logf(resistencia/R25)/BETA;
  if(!isfinite(invT) || invT<=0) return ForaFaixa;
  float calculada=1.0f/invT-273.15f;
  if(!isfinite(calculada) || calculada< -30 || calculada>150) return ForaFaixa;
  graus=calculada; return Ok;
}
class Medidor {
 public:
  int raw=0;
  float graus=0;
  Estado estado=Aguardando;
  bool valido() const { return estado==Ok; }
  void atualizar(int leitura,float dt) {
    raw=leitura; float nova=0; Estado proximo=converter(raw,nova);
    if(proximo!=Ok) { estado=proximo; graus=0; return; }
    if(estado!=Ok) graus=nova;
    else { if(dt<0.001f) dt=0.001f; if(dt>1) dt=1; graus+=(dt/(0.5f+dt))*(nova-graus); }
    estado=proximo;
  }
};
}
