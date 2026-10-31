#pragma once

namespace LimitesTT {
constexpr float ALVO_MIN = 15.0f;
constexpr float ALVO_MAX = 165.0f;
constexpr float CORTE_MIN = 10.0f;
constexpr float CORTE_MAX = 170.0f;
constexpr bool calibracaoValida(int a, int b) {
  return a >= 50 && a <= 4045 && b >= 50 && b <= 4045 &&
         (b - a >= 300 || a - b >= 300);
}
// Graus relativos a dois pontos fisicos separados por NO MAXIMO 180 graus.
// Para um arco menor, a escala nominal abaixo superestima o deslocamento real.
constexpr float graus(int raw, int a, int b) {
  return a == b ? -999.0f : 180.0f * (raw - a) / (b - a);
}
constexpr bool dentro(float pos) { return pos > CORTE_MIN && pos < CORTE_MAX; }
constexpr bool centro(float pos) { return pos >= 75.0f && pos <= 105.0f; }
constexpr int sinal(int x) { return (x > 0) - (x < 0); }
constexpr int deltaLogico(int anterior, int atual, int a, int b) {
  return (atual - anterior) * sinal(b - a);
}
constexpr bool contrario(int anterior, int atual, int a, int b, int esperado) {
  return esperado != 0 && deltaLogico(anterior, atual, a, b) * esperado <= -8;
}
constexpr int polaridade(int delta, int a, int b) {
  return delta == 0 || a == b ? 0 : sinal(delta) * sinal(b - a);
}
}
