#include "../firmware/atual/teste_tt_v7_auto_sem_marcacao/Curso180.h"
using namespace Curso180;
static_assert(!usarManual(false, true, 0, 3550), "Caso da captura: B sozinho nao bloqueia automatico");
static_assert(!usarManual(false, false, 0, 0), "Automatico sem marcacoes");
static_assert(usarManual(true, true, 1000, 3000), "Manual opcional completo");
static_assert(inicioAutomaticoValido(3550), "POS3550 da bancada aceita no automatico");
static_assert(valido(3550-AUTO_MEIO_CURSO, 3550+AUTO_MEIO_CURSO), "Janela automatica valida");
static_assert(centro(3550, 3390, 3710), "Posicao atual e centro automatico");
static_assert(!inicioAutomaticoValido(399) && !inicioAutomaticoValido(3696), "Extremos proximos rejeitados");
static_assert(dentro(3390, 3390, 3710, 12) && dentro(3710, 3390, 3710, 12), "Alvos automaticos permitidos");
static_assert(!dentro(3360, 3390, 3710, 12) && !dentro(3740, 3390, 3710, 12), "Corte da janela automatica");
static_assert(graus(1000,1000,3000)==0 && graus(3000,1000,3000)==180, "Manual ainda disponivel");
static_assert(polaridade(12,1000,3000)==1 && polaridade(-12,1000,3000)==-1, "Sentidos preservados");
