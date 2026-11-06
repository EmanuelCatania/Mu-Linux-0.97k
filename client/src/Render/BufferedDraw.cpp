// BufferedDraw, común a los backends: arma las primitivas del modo inmediato
// del 0.97k (Begin/End, abanicos, quads, strips) y junta las independientes en
// un lote. El backend sólo recibe lotes ya armados (Submit) en el orden de
// dibujo; antes de cada cambio de estado se envía el lote pendiente.
#include "Render/BufferedDraw.h"
#include "Render/Backend/Api.h"
#include "Render/RenderState.h"
#include <cstring>

namespace BufferedDraw {
Stream stream;
namespace {
// Modos de primitiva con la numeración de OpenGL 1.1 (0..9).
enum Mode : unsigned { Points=0, Lines=1, LineLoop=2, LineStrip=3, Triangles=4, TriangleStrip=5, TriangleFan=6, Quads=7, QuadStrip=8, Polygon=9 };
std::vector<Vertex> pending;
unsigned pendingMode=Triangles;
bool ready=false;
void (*exitHook)()=nullptr;
void SetCurrent(const float* color,const float* uv) {
    auto& s=RenderState::Mutable();
    if(color)std::memcpy(s.color,color,sizeof(s.color));
    if(uv)std::memcpy(s.uv,uv,sizeof(s.uv));
}
bool Independent(unsigned mode) { return mode==Triangles||mode==Quads||mode==Lines||mode==Points; }
bool Ready() { return ready || Initialize(); }
}

bool Initialize() {
    if (ready) return true;
    ready=Backend::Initialize();
    return ready;
}
void Release() {
    Suspend();
    stream.active=false;stream.vertices.clear();pending.clear();
    Backend::Release();
    ready=false;
}
void NativeColor(float r,float g,float b,float a) { const float c[4]{r,g,b,a};SetCurrent(c,nullptr); }
void NativeUV(float u,float v) { const float t[4]{u,v,0,1};SetCurrent(nullptr,t); }
// Un vértice fuera de Begin/End no dibuja nada, igual que en OpenGL.
void NativeVertex(float,float,float) {}
Vertex CaptureAttributes(bool color,bool uv){
    Vertex attributes{{},{1,1,1,1},{0,0,0,1}};
    const auto& s=RenderState::Get();
    if(color)std::memcpy(attributes.color,s.color,sizeof(attributes.color));
    if(uv)std::memcpy(attributes.uv,s.uv,sizeof(attributes.uv));
    return attributes;
}
// Primitive no sigue la numeración de OpenGL; traducir a los modos 0..9.
void Begin(Primitive primitive){
    static constexpr unsigned modes[]{Points,Lines,Triangles,Quads,TriangleFan,LineLoop,LineStrip,TriangleStrip,QuadStrip,Polygon};
    Begin(modes[static_cast<unsigned>(primitive)]);
}
void Begin(unsigned mode) {
    if (!Ready()) return;
    stream.mode=mode;stream.vertices.clear();
    const auto& s=RenderState::Get();
    std::memcpy(stream.color,s.color,sizeof(stream.color));std::memcpy(stream.uv,s.uv,sizeof(stream.uv));
    const bool direct=Independent(mode);
    if (direct && !pending.empty() && pendingMode!=mode) Flush();
    if (direct) pendingMode=mode;
    stream.destination=direct ? &pending : &stream.vertices;
    stream.first=stream.destination->size();
    stream.active=true;
}
// DESVIACION: el terreno ya entrega todos los atributos; evitar la captura
// individual de cada esquina. Abanico y quad comparten la diagonal 0-2.
void Quad(const Vertex* corners,bool) {
    if (!Ready()) return;
    if (!pending.empty() && pendingMode!=Quads) Flush();
    pendingMode=Quads;pending.insert(pending.end(),corners,corners+4);
    SetCurrent(corners[3].color,corners[3].uv);
}
void Flush(const Vertex*) {
    if (pending.empty() || !ready) return;
    Backend::Submit(pendingMode,pending.data(),pending.size());
    pending.clear();
}
void SetExitHook(void (*hook)()) { exitHook=hook; }
void Suspend() {
    Flush();
    if(exitHook) { auto hook=exitHook;exitHook=nullptr;hook(); }
    Backend::Suspend();
}
void Activate(unsigned shader,unsigned array,unsigned buffer) {
    Flush();Backend::Activate(shader,array,buffer);
}
void DrawIndexed(unsigned array,unsigned buffer,const Vertex* vertices,std::size_t count,std::size_t indices,const Vertex*) {
    if (!Ready()) return;
    Flush();
    Backend::DrawIndexed(array,buffer,vertices,count,indices);
}

void End() {
    if (!stream.active) return;
    stream.active=false;
    const bool direct=stream.destination==&pending;
    const auto count=stream.destination->size()-stream.first;
    const unsigned mode=stream.mode;
    if (direct) {
        // OpenGL descarta los vértices incompletos al cerrar una primitiva.
        const unsigned group=mode==Triangles?3:mode==Quads?4:mode==Lines?2:1;
        pending.resize(pending.size()-count%group);
    } else if ((mode==TriangleFan||mode==Polygon) && count==4) {
        if (!pending.empty() && pendingMode!=Quads) Flush();
        pendingMode=Quads;pending.insert(pending.end(),stream.vertices.begin(),stream.vertices.end());
    } else if (mode==TriangleFan||mode==Polygon) {
        // Cada abanico conserva sus triángulos y su orden. No se conectan
        // abanicos de tiles distintos al reunirlos en un mismo envío.
        if (!pending.empty() && pendingMode!=Triangles) Flush();
        pendingMode=Triangles;
        for (std::size_t i=1;i+1<count;++i) {
            pending.push_back(stream.vertices[0]);pending.push_back(stream.vertices[i]);pending.push_back(stream.vertices[i+1]);
        }
    } else if (mode==QuadStrip) {
        // Quad i = (2i, 2i+1, 2i+3, 2i+2), con la misma diagonal que Quads.
        if (!pending.empty() && pendingMode!=Quads) Flush();
        pendingMode=Quads;
        for (std::size_t i=0;i+3<count;i+=2) {
            const auto& v=stream.vertices;
            pending.push_back(v[i]);pending.push_back(v[i+1]);pending.push_back(v[i+3]);pending.push_back(v[i+2]);
        }
    } else {
        // Strips y lazos no se concatenan: conservar sus conexiones originales.
        Flush();pendingMode=mode;
        pending.insert(pending.end(),stream.vertices.begin(),stream.vertices.end());
        Flush();
    }
    // El color y la UV actuales quedan en los últimos fijados dentro del bloque.
    SetCurrent(stream.color,stream.uv);
}
}
