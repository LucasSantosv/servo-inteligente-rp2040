#pragma once
namespace Curso180 {
constexpr int PWM = 200; // 78%, corte controlado pela posicao
constexpr float VELOCIDADE_ALVO = 20.0f; // graus/s do alvo, ajustavel apos teste fisico
constexpr float TOLERANCIA = 1.5f;
constexpr int AUTO_MEIO_CURSO = 160;
constexpr bool inicioAutomaticoValido(int raw) { return raw >= 400 && raw <= 3695; }
constexpr int sinal(int x) { return (x > 0) - (x < 0); }
constexpr bool valido(int a, int b) {
  return a >= 100 && a <= 3995 && b >= 100 && b <= 3995 &&
         (b - a >= 300 || a - b >= 300);
}
constexpr float graus(int raw, int a, int b) {
  return a == b ? -999.0f : 180.0f * (raw - a) / (b - a);
}
constexpr bool usarManual(bool temA, bool temB, int a, int b) { return temA && temB && valido(a, b); }
constexpr bool dentro(int raw, int a, int b, float margem = 3.0f) {
  return raw >= 50 && raw <= 4045 && graus(raw, a, b) >= -margem && graus(raw, a, b) <= 180.0f + margem;
}
constexpr bool centro(int raw, int a, int b) {
  return graus(raw, a, b) >= 75.0f && graus(raw, a, b) <= 105.0f;
}
constexpr int polaridade(int delta, int a, int b) {
  return sinal(delta) * sinal(b - a);
}
constexpr bool contrario(int anterior, int atual, int a, int b, int esperado) {
  return esperado != 0 && (atual - anterior) * sinal(b - a) * esperado <= -8;
}
constexpr int pulsoServo(float grausAlvo) { return 1000 + (int)(grausAlvo * (1000.0f / 180.0f) + 0.5f); }
}
