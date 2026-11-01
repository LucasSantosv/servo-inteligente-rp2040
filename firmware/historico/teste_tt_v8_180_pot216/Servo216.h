#pragma once
#include <stdint.h>
#include <math.h>

namespace Servo216 {
// Curso informado pelo usuario. Pressupoe potenciometro linear: 0-3V3 = 0-216 graus.
constexpr float CURSO_POT = 216.0f;
constexpr float A = 18.0f, B = 198.0f;
constexpr float CORTE_A = 9.0f, CORTE_B = 207.0f;
constexpr float VELOCIDADE_ALVO = 15.0f;
constexpr float TOLERANCIA = 1.2f;
constexpr int PWM_DETECCAO = 200, PWM_MAX = 128, PWM_PERTO_PONTA = 96;
constexpr float KP = 9.0f, KD = 1.6f;
constexpr float graus(int raw) { return raw * (CURSO_POT / 4095.0f); }
constexpr int adc(float g) { return (int)(g * (4095.0f / CURSO_POT) + 0.5f); }
constexpr int sinal(float v) { return (v > 0) - (v < 0); }
constexpr float absoluto(float v) { return v < 0 ? -v : v; }
constexpr float limitar(float v, float baixo, float alto) { return v < baixo ? baixo : (v > alto ? alto : v); }
constexpr bool seguro(float g) { return g > CORTE_A && g < CORTE_B; }
constexpr bool partidaValida(float g) { return g >= A + 6 && g <= B - 6; }
constexpr int pulsoServo(float alvo) { return 1000 + (int)(limitar((alvo-A)/(B-A),0,1)*1000+0.5f); }
constexpr float pd(float erro, float velAlvo, float velMedida) { return KP*erro + KD*(velAlvo-velMedida); }
constexpr int limitarSaida(float g, int pwm) {
  return ((g <= A && pwm < 0) || (g >= B && pwm > 0)) ? 0 : pwm;
}

// Mesma malha usada no firmware e nos testes de simulacao. Sem dependencias Arduino.
class Malha {
 public:
  float angulo = 108, velocidade = 0;
  void iniciar(float g, uint32_t agora) {
    angulo = anterior = g; velocidade = 0;
    arranque = false; ultimoArranque = agora - 200;
  }
  void observar(float g, float dt) {
    dt = limitar(dt, 0.001f, 0.05f);
    float instantanea = (g-anterior)/dt;
    velocidade += 0.2f*(instantanea-velocidade);
    angulo = anterior = g;
  }
  int calcular(float alvo, float velAlvo, uint32_t agora) {
    float erro = alvo-angulo;
    if (absoluto(erro) <= 0.7f && absoluto(velocidade) < 5) { arranque=false; return 0; }
    float distanciaPonta = fminf(angulo-A, B-angulo);
    int limite = distanciaPonta < 10 ? PWM_PERTO_PONTA : PWM_MAX;
    int saida = (int)lroundf(limitar(pd(erro,velAlvo,velocidade),-limite,limite));
    // Impulso curto para vencer atrito, em vez de manter 78% continuamente.
    if (!arranque && absoluto(velocidade) < 3 && absoluto(erro) > 1.3f && agora-ultimoArranque >= 200) {
      arranque=true; inicioArranque=ultimoArranque=agora; sentidoArranque=sinal(erro);
    }
    if (arranque) {
      if (agora-inicioArranque >= 12 || absoluto(velocidade) > 60 || erro*sentidoArranque < 0.4f) arranque=false;
      else saida=PWM_DETECCAO*sentidoArranque;
    }
    return limitarSaida(angulo,saida);
  }
 private:
  float anterior = 108;
  bool arranque = false;
  int sentidoArranque = 0;
  uint32_t inicioArranque = 0, ultimoArranque = 0;
};
}
