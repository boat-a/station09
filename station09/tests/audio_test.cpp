#include "../src/audio.hpp"
#include <chrono>
using namespace S9;
int main(){
  auto t0=std::chrono::steady_clock::now();
  audio.init();
  int v=audio.add(voice("Station zero nine, this is surface control. Do you read? Over.",112,0.95f,1.15f,0.12f,0.0f,1));
  int v2=audio.add(voice("Mercer? Is that you? Come to bed.",215,1.22f,1.0f,0.2f,0.3f,3,0.9f,5));
  auto t1=std::chrono::steady_clock::now();
  printf("init %.0f ms\n",std::chrono::duration<double,std::milli>(t1-t0).count());
  int bad=0;
  for(size_t i=0;i<audio.bank.size();i++){ float pk=0,rms=0; for(float x:audio.bank[i]){ if(!(x==x)){bad++;} pk=std::max(pk,fabsf(x)); rms+=x*x;} rms=sqrtf(rms/std::max<size_t>(1,audio.bank[i].size())); printf("%2zu dur %.2f peak %.2f rms %.3f\n",i,audio.bank[i].size()/(float)SR,pk,rms);}
  printf("NaNs: %d\n",bad);
  // render mix to wav
  audio.play(S_DRONE,0.5f,1,nullptr,true); audio.play(v,1.f); V3 s(3,0,2); audio.play(v2,1.f,1,&s);
  int frames=SR*8; std::vector<i16> o(frames*2); for(int i=0;i<frames;i+=1024){ audio.mix(&o[i*2],std::min(1024,frames-i)); }
  FILE*f=fopen("/tmp/mix.raw","wb"); fwrite(o.data(),2,o.size(),f); fclose(f);
  // voice alone raw
  FILE*g=fopen("/tmp/voice.raw","wb"); std::vector<i16> vv; for(float x:audio.bank[v]) vv.push_back((i16)(x*28000)); fwrite(vv.data(),2,vv.size(),g); fclose(g);
}
