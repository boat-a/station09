#include "../src/gfx.hpp"
using namespace S9;
int main(){
  gfx.init();
  gfx.clear(0xFF101010);
  for(int t=0;t<T_COUNT;t++){ int ox=(t%8)*40, oy=(t/8)*40; for(int y=0;y<32;y++)for(int x=0;x<32;x++) gfx.fb[(oy+y+4)*SW+ox+x+4]=gfx.tex[t].px[(y*2)*64+x*2]; }
  gfx.text(4,200,"TEST TEXT: HELLO 0123456789 ?!.,",0xFFFFFFFF,1);
  gfx.text(4,210,"RESTORE THE POWER",0xFFFF4040,2);
  // box test
  gfx.setCam(V3(0,1.5f,-4),0,0);
  LightFn lf=[](const V3&p,const V3&n){return V3(1,1,1);};
  gfx.box(V3(3,1,2),V3(0.5f,1,0.5f),M3::rotY(0.7f),T_CRATE,lf);
  Post pp; gfx.post(pp);
  FILE*f=fopen("/tmp/gfx.ppm","wb"); fprintf(f,"P6\n%d %d\n255\n",SW,SH);
  for(int i=0;i<SW*SH;i++){u32 c=gfx.out[i]; fputc(c>>16&255,f);fputc(c>>8&255,f);fputc(c&255,f);} fclose(f);
}
