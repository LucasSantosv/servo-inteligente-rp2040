#include "../firmware/atual/teste_tt_v15_bancada_carga/ControleB10K.h"
#include "../firmware/atual/teste_tt_v15_bancada_carga/PonteMX1508.h"
#include <cstdio>
#include <cmath>
#include <initializer_list>
using namespace Servo216;
static_assert(ControleB10K::destino(0)==A && ControleB10K::destino(4095)==B,"B10K cobre o arco");
static_assert(ControleB10K::rpm(60)==10 && ControleB10K::rpm(-60)==10,"RPM do eixo monitorado");
int main() {
 int falhas=0;
 {
  ControleB10K::Seguidor s; s.iniciar(108); unsigned rng=12345;
  for(unsigned n=0;n<20000;++n) {
   if(n%50==0) rng=1664525*rng+1013904223;
   float anterior=s.posicao; s.atualizar((rng>>8)%4096,0.005f,false);
   if(s.posicao<A || s.posicao>B || absoluto(s.velocidade)>ControleB10K::VMAX+0.01f || absoluto(s.aceleracao)>ControleB10K::AMAX+0.01f || absoluto(s.posicao-anterior)>ControleB10K::VMAX*0.005f+0.001f) ++falhas;
  }
  for(int n=0;n<4000;++n) s.atualizar(4095,0.005f,false);
  if(absoluto(s.posicao-B)>0.5f) ++falhas;
 }
 {
  Malha m; m.iniciar(108,0);
  for(unsigned ms=5;ms<=100;ms+=5) { m.observar(108,0.005f); m.calcular(198,0,ms,255,true); }
  if(m.saida<250 || m.saida>255) ++falhas;
  if(PonteMX1508::niveis(255,true).a!=256 || PonteMX1508::niveis(255,true).b!=0) ++falhas;
 }
 int casos=0;
 for(bool rapido : {false,true}) for(float vmax : {120.0f,300.0f,600.0f}) for(float tau : {0.03f,0.08f,0.15f}) for(int atrito : {0,50,90,160}) for(int carga : {-25,0,25}) {
  ++casos; Malha m; ControleB10K::Seguidor s; Progresso p;
  float g=108,v=0,ultimoPedido=108; m.iniciar(g,0); s.iniciar(g); p.iniciar(adc(g),0);
  int ultimoPWM=0; unsigned ultimoCorte=0; bool fora=false; float minG=g,maxG=g;
  for(unsigned ms=5;ms<=60000;ms+=5) {
   int raw=ms<20000 ? 4095:(ms<40000 ? 0:2048);
   if(absoluto(ControleB10K::destino(raw)-ultimoPedido)>1.0f) { p.registrarComando(ms); ultimoPedido=ControleB10K::destino(raw); }
   float med=graus(adc(g)); m.observar(med,0.005f); s.atualizar(raw,0.005f,rapido);
   if(!seguro(med)) { fora=true; break; }
   int u=m.calcular(s.posicao,s.velocidade,ms,rapido?255:200,rapido);
   if(std::abs(u)>(rapido?255:200)) { fora=true; break; }
   if(ultimoPWM && (!u || sinal(u)!=sinal(ultimoPWM))) { u=0; ultimoCorte=ms; }
   else if(u && ms-ultimoCorte<5) u=0;
   ultimoPWM=u;
   if(p.travado(adc(g),s.posicao-med,u,0.005f,ms)) { fora=true; break; }
   float esforco=u+carga;
   float vd=absoluto(esforco)<=atrito ? 0:sinal(esforco)*(absoluto(esforco)-atrito)/(255.0f-atrito)*vmax;
   v+=(vd-v)*std::fmin(1.0f,0.005f/tau); g+=v*0.005f;
   minG=std::fmin(minG,g); maxG=std::fmax(maxG,g);
  }
  bool ok=!fora && minG<22 && maxG>194 && absoluto(g-108)<5;
  if(!ok) { ++falhas; std::printf("REVER modo=%c vmax=%.0f tau=%.2f atrito=%d carga=%d arco=%.1f..%.1f fim=%.1f\n",rapido?'f':'p',vmax,tau,atrito,carga,minG,maxG,g); }
 }
 std::printf("Casos B10K: %d; falhas: %d\n",casos,falhas);
 return falhas?1:0;
}


