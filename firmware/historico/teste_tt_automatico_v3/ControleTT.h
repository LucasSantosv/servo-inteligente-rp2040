#pragma once
#include <stdint.h>
namespace ControleTT {
constexpr int CURSO = 120; // ADC para cada lado da posicao inicial
constexpr int MARGEM = 40;
constexpr int PWM = 64; // 25%, aplicado em pulsos curtos
constexpr int PWM_PARTIDA = 200; // 78% somente na deteccao inicial de sentido
constexpr uint32_t CICLO_MS = 50, LIGADO_MS = 15;
constexpr uint32_t CICLO_PARTIDA_MS = 85, PARTIDA_LIGADO_MS = 50;
constexpr int sinal(int x) { return (x > 0) - (x < 0); }
constexpr bool partidaValida(int centro) { return centro >= 1000 && centro <= 3095; }
constexpr bool dentro(int pos, int centro) {
  return pos > centro - CURSO - MARGEM && pos < centro + CURSO + MARGEM;
}
constexpr bool pulso(uint32_t tempo) { return tempo % CICLO_MS < LIGADO_MS; }
constexpr bool pulsoPartida(uint32_t tempo) { return tempo % CICLO_PARTIDA_MS < PARTIDA_LIGADO_MS; }
constexpr bool contrario(int referencia, int atual, int esperado) {
  return esperado != 0 && (atual - referencia) * esperado <= -8;
}
constexpr int alvo(int b10k, int centro) {
  return centro - CURSO + (b10k * (2 * CURSO)) / 4095;
}
}
