#include "../src/game_render.hpp"
using namespace S9;
static Game g; static Input in;
void step(int n,float dt=1/30.f){ for(int i=0;i<n;i++){ g.update(dt,in); g.render(); } }
void save(const char*name){ char p[128]; snprintf(p,128,"/tmp/%s.ppm",name); FILE*f=fopen(p,"wb"); fprintf(f,"P6\n%d %d\n255\n",SW,SH); for(int i=0;i<SW*SH;i++){u32 c=gfx.out[i]; fputc(c>>16&255,f);fputc(c>>8&255,f);fputc(c&255,f);} fclose(f); char cmd[256]; snprintf(cmd,256,"convert %s -scale 300%% /tmp/%s.png",p,name); system(cmd);}
void tp(int lvl,float x,float z,float yaw,float pitch=0){ g.P.cur=lvl; g.pos=V3(x,0,z); g.yaw=yaw; g.pitch=pitch; }
int main(){
  g.init(); in.key[K_ENTER]=true; step(1); in.key[K_ENTER]=false; step(2);
  for(int i=0;i<8;i++){ in.key[K_SPACE]=true; step(1); in.key[K_SPACE]=false; step(3);} 
  g.mimic.despawn(9999); g.flashOn=true;
  g.ui=UI_COMMS; g.tuneFreq=95.f; g.tuneTarget=96.2f; g.aStatic=0; step(3); save("u_comms");
  g.ui=UI_REPORT; g.repStage=0; step(2); save("u_report");
  g.repStage=2; g.P.reportPick[0][0]=0; g.P.reportPick[0][1]=1; g.repT=0; step(2); save("u_report2");
  g.ui=UI_SECPC; g.codeIn="4713"; g.uiMsg="ACCESS DENIED"; step(2); save("u_sec");
  g.ui=UI_CORE; step(2); save("u_core");
  g.ui=UI_NOTE; g.noteShown=4; step(2); save("u_note");
  g.P.tape[0]=true; g.P.tape[2]=true; g.P.reportDone[0]=true; g.ui=UI_JOURNAL; step(2); save("u_journal");
  g.ui=UI_NONE; 
  // locker hide
  g.P.hasPistol=true; g.P.ammo=4; 
  tp(0,19.5f,2.5f,PI*0.5f);
  Obj*lk=nullptr; for(auto&o:g.L().objs) if(o.type==O_LOCKER&&o.pos.x>20){lk=&o;break;}
  V3 f(sinf(lk->yaw),0,cosf(lk->yaw)); g.pos=lk->pos+f*0.9f; g.yaw=atan2f(-f.x,-f.z); step(2); in.key['E']=true; step(1); in.key['E']=false; step(3); save("u_hide");
  in.key['E']=true; step(1); in.key['E']=false; step(3);
  tp(0,10.5f,8.8f,PI*0.5f); g.flashOn=true; in.mouse[0]=true; step(1); in.mouse[0]=false; g.recoil=0.0f; step(1); save("u_gun");
  g.js=0.5f; g.jsType=0; step(1); save("u_js");
  // chamber cutscene
  g.P.coreDone=false; tp(2,14.5f,9.5f,0); step(2); for(int i=0;i<50;i++) step(1); save("c_chamber1"); for(int i=0;i<60;i++) step(1); save("c_chamber2");
  // death screens
  g.mode=M_PLAY; g.die(); for(int i=0;i<20;i++) step(1); save("d_face"); for(int i=0;i<100;i++) step(1); save("d_text");
  // endings
  g.mode=M_END; g.endingKind=1; g.endT=7; g.playTime=1432; step(1); save("e_good");
  g.endingKind=2; step(1); save("e_bad");
  // elevator, ready
  g.mode=M_PLAY; g.ui=UI_NONE; g.P.cur=2; g.P.genOn=true; g.P.tripped=false; g.P.elevCalled=true; g.P.elevTimer=12; g.P.coreDone=true; g.hurt=0;
  { Obj*e=nullptr; for(auto&o:g.L().objs) if(o.type==O_ELEV) e=&o; V3 ff(sinf(e->yaw),0,cosf(e->yaw)); g.pos=e->pos+ff*2.0f; g.yaw=atan2f(-ff.x,-ff.z); g.flashOn=false; }
  g.mimic.spawnAt(V3(11.5f,0,22.5f),MS_PATROL); step(3); save("p_elev");
}
