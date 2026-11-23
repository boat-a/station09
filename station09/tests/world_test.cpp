#include "../src/world.hpp"
using namespace S9;
int main(){
  const char* names[3]={"B1","B2","B3"};
  for(int L=0;L<3;L++){
    static Level lv; lv.load(L);
    const char* const* rows = L==0?MAP_B1:(L==1?MAP_B2:MAP_B3); int n=L==0?sizeof(MAP_B1)/8:(L==1?sizeof(MAP_B2)/8:sizeof(MAP_B3)/8);
    for(int i=0;i<n;i++) if((int)strlen(rows[i])!=lv.W) printf("!! %s row %d len %zu != %d\n",names[L],i,strlen(rows[i]),lv.W);
    printf("%s: %dx%d objs %zu doors %zu lights %zu trigs %zu wps %zu quads ",names[L],lv.W,lv.H,lv.objs.size(),lv.doors.size(),lv.lights.size(),lv.trigs.size(),lv.waypoints.size());
    size_t q=0; for(auto&c:lv.cq) q+=c.size(); printf("%zu\n",q);
    if(L==2) lv.start=V3(14.5f,0,2.5f); std::vector<short> d; lv.bfs((int)lv.start.x,(int)lv.start.z,d);
    if(L==0) printf("start %.1f %.1f\n",lv.start.x,lv.start.z);
    for(auto&o:lv.objs){ int cx=(int)o.pos.x,cz=(int)o.pos.z; // adjacent reachable?
      int best=9999; for(int dz=-2;dz<=2;dz++)for(int dx=-2;dx<=2;dx++){int nx=cx+dx,nz=cz+dz; if(nx<0||nz<0||nx>=lv.W||nz>=lv.H)continue; best=std::min<int>(best,d[nz*lv.W+nx]);}
      if(best>=9999) printf("  UNREACHABLE obj type %d glyph %c at %d,%d\n",o.type,o.glyph,cx,cz); }
    for(auto&w:lv.waypoints){ if(d[(int)w.z*lv.W+(int)w.x]>=9999) printf("  unreachable waypoint %.0f,%.0f\n",w.x,w.z);}
    for(auto&t:lv.trigs){ if(d[t.cz*lv.W+t.cx]>=9999) printf("  unreachable trig %d at %d,%d\n",t.id,t.cx,t.cz);}
  }
}
