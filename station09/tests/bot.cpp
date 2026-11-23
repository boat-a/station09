// End-to-end bot: plays through the whole game using real input paths and checks every milestone.
#include "../src/game_render.hpp"
using namespace S9;
static Game g; static Input in;
static int fails=0;
#define CHECK(c,msg) do{ if(!(c)){ printf("  FAIL: %s (line %d)\n",msg,__LINE__); fails++; } else printf("  ok: %s\n",msg);}while(0)
void step(int n,float dt=1/30.f){ for(int i=0;i<n;i++){ g.P.health=std::max(g.P.health,100.f); g.update(dt,in); g.render(); } }
void secs(float s){ step((int)(s*30)); }
void press(int k,int fr=2){ in.key[k&255]=true; step(1); in.key[k&255]=false; step(fr-1); }
Obj* find(char glyph,int lvl=-1){ Level&l=lvl<0?g.L():g.lv[lvl]; for(auto&o:l.objs) if(o.glyph==glyph&&!o.taken) return &o; return nullptr; }
void stand(Obj*o,float d=1.1f){ V3 f(sinf(o->yaw),0,cosf(o->yaw)); g.pos=o->pos+f*d; g.pos.y=0; g.yaw=atan2f(-f.x,-f.z); g.pitch=0; g.hidden=false; g.ui=UI_NONE; step(2); }
bool use(char glyph,float d=1.1f){ Obj*o=find(glyph); if(!o){ printf("  (no obj %c)\n",glyph); return false;} stand(o,d); press('E'); return true; }
void holdE(float s){ in.key['E']=true; secs(s); in.key['E']=false; step(2); }
void skipCuts(){ int guard=0; while(g.mode==M_CUT && guard++<40){ press(K_SPACE,3); } }
void waitMode(Mode m,float maxs){ for(int i=0;i<(int)(maxs*30)&&g.mode!=m;i++) step(1); }
void goStairs(char glyph){ Obj*o=find(glyph); stand(o,1.0f); press('E'); secs(2.0f); skipCuts(); step(5); }
void cell(int cx,int cz){ g.pos=V3(cx+0.5f,0,cz+0.5f); step(2); }

int run(bool purge){
  printf("\n=== RUN: %s ===\n",purge?"PURGE (good ending)":"LEAVE (bad ending)");
  g.newGame(); step(1);
  g.mimic.despawn(9999); // keep the bot alive & deterministic; AI is exercised in separate test
  skipCuts();
  CHECK(g.mode==M_PLAY,"intro ends in play mode");
  // --- B1 ---
  CHECK(g.P.cur==0,"start on B1");
  // stairs should be refused before comms
  { Obj*o=find('v'); stand(o,1.0f); press('E'); secs(1.5f); CHECK(g.P.cur==0,"stairs refused before comms check"); }
  // report 1
  use('R'); CHECK(g.ui==UI_REPORT,"report UI opens");
  press('1'); press('2'); for(int i=0;i<40*30&&!g.P.reportDone[0];i++) step(1); CHECK(g.P.reportDone[0],"report 1 filed");
  // comms
  use('n'); CHECK(g.ui==UI_COMMS,"comms UI opens");
  g.tuneFreq=g.tuneTarget; secs(2.5f); CHECK(g.P.commsDone,"comms tuned");
  secs(14.f);
  // pickups on B1
  use('f'); CHECK(g.P.fuseHeld[0],"fuse 1 taken");
  use('t'); CHECK(g.P.tape[0],"tape 1 taken"); secs(1);
  use('b'); use('m');
  cell(10,9); cell(24,9); // trigger 1 & 4
  CHECK(g.P.sighted,"first sighting trigger fired");
  g.mimic.despawn(9999);
  goStairs('v'); CHECK(g.P.cur==1,"descended to B2");
  g.mimic.despawn(9999);
  // --- B2 ---
  use('j'); use('h'); CHECK(g.P.fuseHeld[1]&&g.P.fuseHeld[2],"fuses 2 and 3 taken");
  use('u'); CHECK(g.P.tape[1],"tape 2 taken"); secs(1);
  use('R'); press('2'); press('1'); for(int i=0;i<40*30&&!g.P.reportDone[1];i++) step(1); CHECK(g.P.reportDone[1],"report 2 filed");
  { Obj*o=find('Y'); Obj*x=find('X'); stand(x,1.1f); press('E'); CHECK(g.msg.find("GENERATOR")==std::string::npos,"fusebox no error"); holdE(2.3f); CHECK(g.P.fuseInstalled,"fuses installed (hold E)"); (void)o; }
  { Obj*o=find('Y'); stand(o,1.5f); holdE(3.4f); CHECK(g.mode==M_CUT && g.csId==CS_GEN,"generator cutscene starts"); skipCuts(); CHECK(g.P.genOn,"generator on after cutscene"); secs(8.f); }
  CHECK(g.power(),"power is on");
  g.mimic.despawn(9999);
  // power trip near security door
  cell(28,10); CHECK(g.P.tripped,"power trips near the security door");
  { Door*d=g.L().doorAtCell(29,9); g.pos=V3(29.5f,0,10.5f); g.yaw=0; g.pitch=0; g.yaw=PI; press('E'); CHECK(d->target<0.5f,"powered door refuses while tripped"); }
  { Obj*b=find('B'); stand(b,1.0f); holdE(3.4f); CHECK(!g.P.tripped,"breaker reset restores power"); }
  g.mimic.despawn(9999);
  { Door*d=g.L().doorAtCell(29,9); g.pos=V3(29.5f,0,10.5f); g.yaw=PI; g.pitch=0; step(2); press('E'); secs(1.5f); CHECK(d->open>0.7f,"security office door opens"); }
  use('q'); CHECK(g.P.tape[2],"tape 3 taken"); secs(1);
  use('p'); CHECK(g.P.hasPistol,"pistol taken");
  use('a');
  { Obj*n=find('N'); stand(n,1.0f); press('E'); CHECK(g.ui==UI_SECPC,"security computer UI opens");
    for(char c:std::string("123456")) press(c); press(K_ENTER); CHECK(!g.P.secUnlocked,"wrong code denied");
    for(char c:std::string(SECURITY_CODE)) press(c); press(K_ENTER); CHECK(g.P.secUnlocked,"correct code accepted"); skipCuts(); }
  g.mimic.despawn(9999);
  { Door*d=g.L().doorAtCell(33,12); g.pos=V3(33.5f,0,10.5f); g.yaw=0; g.pos.z=11.0f; step(3); (void)d; // walk up
    g.pos=V3(33.5f,0,12.5f); step(30); CHECK(g.L().doorAtCell(33,12)->open>0.7f,"restricted door opens after code"); }
  goStairs('V'); CHECK(g.P.cur==2,"descended to B3");
  skipCuts(); CHECK(g.mode==M_PLAY,"B3 arrival cutscene ends");
  g.mimic.despawn(9999);
  // --- B3 ---
  CHECK(g.L().doorAtCell(15,17)->jammed,"chamber south door jammed until core is decided");
  cell(14,9); CHECK(g.mode==M_CUT&&g.csId==CS_CHAMBER,"chamber cutscene triggers"); skipCuts(); g.mimic.despawn(9999);
  use('R'); press('1'); press('1'); for(int i=0;i<40*30&&!g.P.reportDone[2];i++) step(1); CHECK(g.P.reportDone[2],"report 3 filed");
  { Obj*q=find('Q'); stand(q,1.3f); press('E'); CHECK(g.ui==UI_CORE,"core terminal opens"); press(purge?'1':'2'); if(purge){ skipCuts(); } step(3); }
  CHECK(g.P.coreDone,"core decision made"); CHECK(g.P.purged==purge,"purge flag matches choice");
  CHECK(!g.L().doorAtCell(15,17)->jammed,"doors unjammed after core");
  g.mimic.despawn(9999);
  cell(14,22); CHECK(g.P.tripped,"power trips at the elevator lobby");
  { Obj*e=find('E'); stand(e,1.3f); press('E'); CHECK(!g.P.elevCalled,"elevator needs power"); }
  { Obj*b=find('B'); stand(b,1.0f); holdE(3.4f); CHECK(!g.P.tripped,"B3 breaker reset"); }
  g.mimic.despawn(9999);
  { Obj*e=find('E'); stand(e,1.3f); holdE(2.4f); CHECK(g.P.elevCalled,"elevator called"); }
  g.mimic.despawn(9999);
  for(int i=0;i<60*30 && !g.P.elevReady;i++){ g.mimic.despawn(9999); g.P.health=100; step(1); }
  CHECK(g.P.elevReady,"elevator arrives after countdown");
  { Obj*e=find('E'); stand(e,1.3f); press('E'); CHECK(g.mode==M_CUT,"ending cutscene starts"); }
  waitMode(M_END,60);
  CHECK(g.mode==M_END,"reached the ending screen");
  CHECK(g.endingKind==(purge?1:2),"correct ending selected");
  for(int i=0;i<30*7;i++) step(1);
  save_shot:
  { char p[64]; snprintf(p,64,"/tmp/end%d.ppm",purge?1:2); FILE*f=fopen(p,"wb"); fprintf(f,"P6\n%d %d\n255\n",SW,SH); for(int i=0;i<SW*SH;i++){u32 c=gfx.out[i]; fputc(c>>16&255,f);fputc(c>>8&255,f);fputc(c&255,f);} fclose(f); }
  return fails;
}

void aiTest(){
  printf("\n=== AI / death / retry test ===\n");
  g.newGame(); step(1); skipCuts();
  g.P.sighted=true; g.P.cur=1; g.arrive(1,0); g.P.visited[1]=true; step(2);
  // let the thing spawn and roam for a few minutes with the player standing still & making noise
  int hunts=0, attacks=0; bool sawMove=false; V3 lastP;
  g.P.health=100; g.mimic.despawn(1.f); g.mimic.aggression=1.f;
  g.pos=V3(5.5f,0,10.5f);
  float lastHealth=100;
  #ifdef QUICK
  const int AIF=30*45;
#else
  const int AIF=30*240;
#endif
  for(int i=0;i<AIF;i++){
    g.update(1/30.f,in); g.render();
    if(g.mode==M_DEAD){ attacks++; break; }
    if(g.mimic.state==MS_HUNT) hunts++;
    if(g.P.health<lastHealth) attacks++;
    lastHealth=g.P.health;
    if(i%90==0){ g.makeNoise(g.pos,12.f); }
    if(vlen(g.mimic.pos-lastP)>0.01f) sawMove=true; lastP=g.mimic.pos;
    for(int k=0;k<3;k++) if(!(g.mimic.pos.x==g.mimic.pos.x)) { printf("  FAIL: NaN position\n"); fails++; return; }
  }
  printf("  hunt frames %d, attacks %d, mode %d, mimic moved: %d\n",hunts,attacks,(int)g.mode,(int)sawMove);
  CHECK(sawMove,"mimic roams");
  CHECK(attacks>0 || hunts>0,"mimic notices and attacks a noisy, stationary player");
  // force a death
  if(g.mode!=M_DEAD){ g.P.health=1; g.mimic.spawnAt(g.pos+V3(1.5f,0,0),MS_HUNT); g.mimic.attackCd=0; for(int i=0;i<90&&g.mode!=M_DEAD;i++){ g.update(1/30.f,in); g.render(); g.P.health=std::min(g.P.health,1.f);} }
  CHECK(g.mode==M_DEAD,"player can die");
  for(int i=0;i<30*4;i++){ g.update(1/30.f,in); g.render(); }
  in.key[K_ENTER]=true; step(1); in.key[K_ENTER]=false; step(2);
  CHECK(g.mode==M_PLAY,"retry returns to play from checkpoint");
  // locker hide test
  g.mimic.despawn(9999);
  Obj* lk=nullptr; for(auto&o:g.L().objs) if(o.type==O_LOCKER){ lk=&o; break; }
  if(lk){ stand(lk,0.9f); press('E'); CHECK(g.hidden,"hides in locker"); press('E'); CHECK(!g.hidden,"leaves locker"); }
  // pistol stagger
  g.P.hasPistol=true; g.P.ammo=5; g.pos=V3(10.5f,0,10.5f); g.yaw=PI*0.5f; g.pitch=0;
  g.mimic.spawnAt(V3(14.5f,0,10.5f),MS_PATROL); step(1); g.mimic.pos=V3(14.5f,0,10.5f);
  g.pitch=atan2f(-(1.5f-1.3f),4.f); in.mouse[0]=true; step(1); in.mouse[0]=false; step(1);
  CHECK(g.mimic.state==MS_STAGGER||g.mimic.hits>0,"pistol staggers the mimic");
}

int main(){
  g.init();
  run(true);
  int f1=fails;
#ifndef QUICK
  run(false);
#endif
  aiTest();
  printf("\n%s (%d failures)\n",fails?"SOME TESTS FAILED":"ALL TESTS PASSED",fails);
  (void)f1;
  return fails?1:0;
}
