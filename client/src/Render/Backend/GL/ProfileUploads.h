#pragma once
#include "Render/RenderProfile.h"

// Sólo diagnóstico: bytes de carga lógica, separados de reservas sin datos.
// No son memoria residente ni tiempo de ejecución GPU. Se consulta el nombre
// ligado únicamente con el perfil encendido; no se modifica ningún estado GL.
namespace ProfileUploadGL {
inline std::uint64_t PixelBytes(unsigned format,unsigned type) {
    switch(type) {
    case 0x8032: case 0x8362:return 1; // packed 3_3_2
    case 0x8033: case 0x8034: case 0x8363: case 0x8364: case 0x8365: case 0x8366:return 2;
    case 0x8035: case 0x8036: case 0x8367: case 0x8368: case 0x84FA:return 4;
    case 0x8DAD:return 8;
    }
    unsigned channels=0;
    switch(format){case 0x1903:case 0x1904:case 0x1905:case 0x1906:case 0x1909:case 0x1902:case 0x1901:channels=1;break;
    case 0x8227:case 0x190A:channels=2;break;case 0x1907:case 0x80E0:channels=3;break;case 0x1908:case 0x80E1:channels=4;break;}
    unsigned component=0;
    switch(type){case 0x1400:case 0x1401:component=1;break;case 0x1402:case 0x1403:case 0x140B:component=2;break;case 0x1404:case 0x1405:case 0x1406:component=4;break;}
    return std::uint64_t(channels)*component;
}
inline void Texture(unsigned target,int w,int h,int d,unsigned format,unsigned type,const void* data,bool reserve) {
    if(!RenderProfile::active || w<0 || h<0 || d<0)return;
    int binding=0;unsigned query=0;
    switch(target){case 0x0DE0:query=0x8068;break;case 0x0DE1:query=0x8069;break;case 0x806F:query=0x806A;break;case 0x8C1A:query=0x8C1D;break;default:if(target>=0x8515 && target<=0x851A)query=0x8514;}
    if(!query)return; // Los proxy no transfieren datos.
    ::glGetIntegerv(query,&binding);
    const auto bytes=std::uint64_t(w)*h*d*PixelBytes(format,type);
    int pbo=0;::glGetIntegerv(0x88EF,&pbo);
    RenderProfile::TextureUpload(static_cast<unsigned>(binding),(data || pbo)?bytes:0,reserve?bytes:0);
}
template<class F> inline void Image2D(F function,unsigned target,int level,int internal,int w,int h,int border,unsigned format,unsigned type,const void* data){
    Texture(target,w,h,1,format,type,data,true);function(target,level,internal,w,h,border,format,type,data);
}
template<class F> inline void Sub2D(F function,unsigned target,int level,int x,int y,int w,int h,unsigned format,unsigned type,const void* data){
    Texture(target,w,h,1,format,type,data,false);function(target,level,x,y,w,h,format,type,data);
}
template<class F> inline void Image3D(F function,unsigned target,int level,int internal,int w,int h,int d,int border,unsigned format,unsigned type,const void* data){
    Texture(target,w,h,d,format,type,data,true);function(target,level,internal,w,h,d,border,format,type,data);
}
template<class F> inline void Sub3D(F function,unsigned target,int level,int x,int y,int z,int w,int h,int d,unsigned format,unsigned type,const void* data){
    Texture(target,w,h,d,format,type,data,false);function(target,level,x,y,z,w,h,d,format,type,data);
}
template<class F,class S> inline void BufferData(F function,unsigned target,S bytes,const void* data,unsigned usage){
    if(RenderProfile::active && bytes>=0)RenderProfile::BufferUpload(data?static_cast<std::uint64_t>(bytes):0,static_cast<std::uint64_t>(bytes));function(target,bytes,data,usage);
}
template<class F,class O,class S> inline void BufferSubData(F function,unsigned target,O offset,S bytes,const void* data){
    if(RenderProfile::active && bytes>=0)RenderProfile::BufferUpload(data?static_cast<std::uint64_t>(bytes):0);function(target,offset,bytes,data);
}
}
