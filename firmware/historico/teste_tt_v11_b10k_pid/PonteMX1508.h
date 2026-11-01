#pragma once
#include <stdint.h>
namespace PonteMX1508 {
// MX1508 Rev1.3, p.10, PWM modo B: alterna acionamento e freio, nao alta impedancia.
// Contador PWM 0..255: nivel 256 significa entrada permanentemente alta.
struct Niveis { uint16_t a,b; };
constexpr int absInt(int v) { return v<0 ? -v:v; }
constexpr Niveis niveis(int comando,bool habilitada) {
  int magnitude=absInt(comando)>255 ? 255:absInt(comando);
  int ligado=(magnitude*256+127)/255;
  return !habilitada ? Niveis{0,0} :
         (comando>=0 ? Niveis{256,(uint16_t)(256-ligado)} : Niveis{(uint16_t)(256-ligado),256});
}
}
