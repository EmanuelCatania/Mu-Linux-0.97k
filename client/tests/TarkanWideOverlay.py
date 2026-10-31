#!/usr/bin/env python3
"""Arena de Tarkan: geometría real contra el ortho de ventana y UV originales."""
import argparse,tempfile
from pathlib import Path
from SceneObjectPointer import function,read_source,build_and_run
PRE=r'''
#include <array>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
void Require(bool ok){if(!ok){std::fputs("Arena fuera del viewport o UV alteradas\n",stderr);std::exit(1);}}
struct Window{int width,height,real,offset;int GetWidth(){return width;}int GetHeight(){return height;}int GetRealWidth(){return real;}int GetUiOffsetX(){return offset;}}gWindow;
struct Map{int id=8;int GetCurrentMap(){return id;}}gMapManager;
float WorldTime,_DAT_00552b88=.0002f,_DAT_00552500=.001f;
void GL_SetBlendSrcOver(char){}void GL_SetBlendAdditive(){}
int texture;void GL_BindTextureSlot(int id){texture=id;}
struct Vertex{int texture;float x,y,u,v;};std::vector<Vertex> vertices;
namespace BufferedDraw{enum class Primitive{TriangleFan};float u,v;unsigned begins,ends;void Begin(Primitive p){Require(p==Primitive::TriangleFan);++begins;}void End(){++ends;}void Color3f(float,float,float){}void Color4f(float,float,float,float){}void TexCoord2f(float a,float b){u=a;v=b;}void Vertex2f(float x,float y){vertices.push_back({texture,x,y,u,v});}}
'''
MAIN=r'''
int main(){unsigned cases=0;const int sizes[][4]={{640,480,640,0},{800,600,800,0},{1280,1024,1280,0},{1600,900,1200,200},{1920,1080,1440,240},{2560,1080,1440,560}};
 for(const auto& s:sizes)for(float time:{0.f,12345.5f,99999.f,100001.f}){gWindow={s[2],s[1],s[0],s[3]};WorldTime=time;vertices.clear();BufferedDraw::begins=BufferedDraw::ends=0;Draw();Require(vertices.size()==8&&BufferedDraw::begins==2&&BufferedDraw::ends==2);
  for(unsigned layer=0;layer<2;++layer){const float scroll=float(static_cast<long long>(time)%100000)*(layer?.001f:.0002f),uw=layer?3.f:.30000001f,vh=layer?2.f:.30000001f;const float left=-float(s[3]),right=float(s[0]-s[3]),top=float(s[1]),bottom=top-435.f*(float(s[1])/480.f);const float x[]={left,left,right,right},y[]={top,bottom,bottom,top},u[]={scroll,scroll,scroll+uw,scroll+uw},v[]={vh*.25f,vh-vh*.25f,vh,0};
   for(unsigned corner=0;corner<4;++corner){const auto& got=vertices[layer*4+corner];Require(got.texture==int(0x494+layer)&&got.x==x[corner]&&got.y==y[corner]&&got.u==u[corner]&&got.v==v[corner]);}
  }++cases;
 }gMapManager.id=0;vertices.clear();Draw();Require(vertices.empty());std::printf("TarkanWideOverlay: %u pares de capas, viewport/UV y otros mapas OK\n",cases);
}
'''
def harness():
    source=read_source('src/Render/Render_Frame.cpp');start=source.index('    if (gMapManager.GetCurrentMap() == 8)');end=source.index('    Render_CharInfoPanel();',start)
    return PRE+''.join(function(source,n)for n in ['ConvertX_RF','ConvertY_RF','RenderBitmapUV'])+'void Draw(){'+source[start:end]+'}'+MAIN
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');a=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-tarkan-wide-')as t:print(build_and_run(Path(t)/'current',harness(),a),end='')
if __name__=='__main__':main()
