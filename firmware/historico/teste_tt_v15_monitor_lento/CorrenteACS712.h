#pragma once
#include <stdint.h>

// ACS712 a 5 V, OUT com divisor 10k/10k no ADC de 3,3 V.
// Apenas telemetria: nao modifica PWM nem aplica corte por corrente.
namespace CorrenteACS712 {
constexpr float VREF_MV=3300.0f, DIVISOR=2.0f;
class Medidor {
 public:
  int raw=0, variante=0;
  float filtrado=0, zero=0;
  bool calibrado=false, zerando=false;
  void selecionar(int codigo) { variante=(codigo>=0 && codigo<=3)?codigo:0; }
  float sensibilidade() const { return variante==1?185.0f:variante==2?100.0f:variante==3?66.0f:0; }
  bool sinalValido() const { return raw>4 && raw<4091; }
  bool zeroValido() const { return calibrado && sinalValido() && !zerando; }
  bool amperesValidos() const { return zeroValido() && sensibilidade()>0; }
  float deltaMV() const { return zeroValido()?(filtrado-zero)*VREF_MV*DIVISOR/4095.0f:0; }
  float miliamperes() const { return amperesValidos()?deltaMV()*1000.0f/sensibilidade():0; }
  void iniciarZero(uint32_t agora) {
    calibrado=false; zerando=true; inicio=agora; soma=0; amostras=0; minimo=4095; maximo=0;
  }
  void cancelarZero() { zerando=false; }
  // Chamar uma vez por ciclo de 5 ms, inclusive com o motor parado.
  // Retorno: 0 em curso, 1 zero aceito, -1 zero rejeitado.
  int atualizar(int leitura,float dt,uint32_t agora) {
    raw=leitura;
    if(!iniciado) { filtrado=raw; iniciado=true; }
    if(dt<0.001f) dt=0.001f;
    if(dt>0.05f) dt=0.05f;
    filtrado+=(dt/(0.030f+dt))*(raw-filtrado);
    if(!zerando || agora-inicio<500) return 0;
    soma+=raw; ++amostras;
    if(raw<minimo) minimo=raw;
    if(raw>maximo) maximo=raw;
    if(amostras<128) return 0;
    zerando=false;
    float candidato=soma/128.0f;
    float mvADC=candidato*VREF_MV/4095.0f;
    calibrado=mvADC>=900 && mvADC<=1600 && maximo-minimo<=100;
    if(calibrado) { zero=candidato; filtrado=zero; }
    return calibrado?1:-1;
  }
 private:
  bool iniciado=false;
  uint32_t inicio=0,soma=0;
  int amostras=0,minimo=4095,maximo=0;
};
}
