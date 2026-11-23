#include "../src/game_render.hpp"
#include <chrono>
using namespace S9;
static Game g; static Input in;
void step(int n,float dt=1/30.f){ for(int i=0;i<n;i++){ g.update(dt,in); g.render(); } }
void save(const char*name){ char p[128]; snprintf(p,128,"/tmp/%s.ppm",name); FILE*f=fopen(p,"wb"); fprintf(f,"P6\n%d %d\n255\n",SW,SH); for(int i=0;i<SW*SH;i++){u32 c=gfx.out[i]; fputc(c>>16&255,f);fputc(c>>8&255,f);fputc(c&255,f);} fclose(f); char cmd[256]; snprintf(cmd,256,"convert %s -scale 300%% /tmp/%s.png",p,name); system(cmd);}
void tp(int lvl,float x,float z,float yaw,float pitch=0){ g.P.cur=lvl; g.pos=V3(x,0,z); g.yaw=yaw; g.pitch=pitch; }
int main(){
  g.init();
  step(30); save("s_title");
  in.key[K_ENTER]=true; step(1); in.key[K_ENTER]=false;
  step(30*2); save("s_intro1");
  step(30*10); save("s_intro2");
  step(30*6); save("s_intro3");
  in.key[K_SPACE]=true; step(2); in.key[K_SPACE]=false; step(2);
  printf("mode after intro: %d\n",(int)g.mode);
  g.flashOn=true;
  tp(0,5.5f,3.5f,PI*0.5f); step(5); save("s_office");
  tp(0,6.5f,8.8f,PI*0.5f); step(5); save("s_corridor");
  tp(0,19.5f,6.0f,0.0f); g.P.cur=0; step(3); save("s_break");
  g.P.genOn=true; tp(0,6.5f,8.8f,PI*0.5f); step(5); save("s_corridor_pow");
  g.P.genOn=false;
  tp(1,6.5f,6.0f,PI,0.0f); step(5); save("s_gen");
  tp(1,20.5f,6.0f,0.0f); step(5); save("s_pump");
  tp(2,15.5f,9.8f,0.0f); step(5); save("s_chamber");
  tp(2,14.5f,21.0f,0.0f); step(5); save("s_lobby");
  // spawn mimic in view
  g.mimic.spawnAt(V3(14.5f,0,16.0f),MS_SCRIPT); g.mimic.yaw=PI; tp(2,14.5f,21.5f,PI); step(2); 
  // timing
  auto t0=std::chrono::steady_clock::now(); for(int i=0;i<60;i++){ g.update(1/30.f,in); g.render(); } auto t1=std::chrono::steady_clock::now();
  printf("avg frame %.2f ms\n",std::chrono::duration<double,std::milli>(t1-t0).count()/60);
  tp(2,14.5f,13.5f,PI*0.0f); g.mimic.spawnAt(V3(14.5f,0,17.0f),MS_SCRIPT); step(2); save("s_mimic");
  g.mimic.state=MS_HUNT; g.mimic.pos=V3(14.5f,0,15.5f); g.mimic.moving=true; step(1); save("s_mimic_hunt");
  g.js=0.7f; step(1); save("s_js");
}
