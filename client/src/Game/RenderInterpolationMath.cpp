#include "stdafx.h"
#include "Game/RenderInterpolationMath.h"
#include <cmath>
#include <cstring>

float RenderBlend(float a, float b, float t) {
    return t<=0 ? a : t>=1 ? b : a+(b-a)*t;
}
float RenderBlendAngle(float a, float b, float t) {
    if (t<=0) return a;
    if (t>=1) return b;
    // Los objetos quietos no requieren dos fmod por ángulo en cada draw.
    // Sólo el intervalo ya normalizado conserva exactamente el resultado;
    // los demás valores siguen el camino general (incluidos 360 y negativos).
    if (a==b && a>=0 && a<360 && std::isfinite(t)) return a+0.0f;
    // DESVIACION: fmod es identidad dentro de (-360, 360); conservar
    // el camino general en los bordes y para valores no finitos.
    float delta=b-a;
    if (!(delta> -360 && delta<360)) delta=std::fmod(delta,360.0f);
    if (delta>180) delta-=360;
    if (delta< -180) delta+=360;
    float angle=a+delta*t;
    if (!(angle> -360 && angle<360)) angle=std::fmod(angle,360.0f);
    return angle<0 ? angle+360 : angle;
}
namespace {
struct Rotation { float q[4], scale[3]; };
bool Decompose(const float *m, Rotation &r) {
    float a[9];
    for (int column=0;column<3;++column) {
        const float length=std::sqrt(m[column]*m[column]+m[4+column]*m[4+column]+m[8+column]*m[8+column]);
        if (!(length>0)) return false;
        r.scale[column]=length;
    }
    const float determinant=m[0]*(m[5]*m[10]-m[6]*m[9])-m[1]*(m[4]*m[10]-m[6]*m[8])+m[2]*(m[4]*m[9]-m[5]*m[8]);
    if (determinant<0) r.scale[0]=-r.scale[0];
    for (int row=0;row<3;++row) for (int col=0;col<3;++col) a[row*3+col]=m[row*4+col]/r.scale[col];
    const float trace=a[0]+a[4]+a[8];
    if (trace>0) {
        const float s=std::sqrt(trace+1)*2;
        r.q[3]=s*.25f; r.q[0]=(a[7]-a[5])/s; r.q[1]=(a[2]-a[6])/s; r.q[2]=(a[3]-a[1])/s;
    } else {
        int i=a[4]>a[0] ? 1 : 0;
        if (a[8]>a[i*3+i]) i=2;
        const int j=(i+1)%3,k=(i+2)%3;
        const float s=std::sqrt(1+a[i*3+i]-a[j*3+j]-a[k*3+k])*2;
        if (!(s>0)) return false;
        r.q[i]=s*.25f; r.q[j]=(a[j*3+i]+a[i*3+j])/s; r.q[k]=(a[k*3+i]+a[i*3+k])/s;
        r.q[3]=(a[k*3+j]-a[j*3+k])/s;
    }
    return true;
}
}
void RenderBlendMatrix(const float *a, const float *b, float t, float *out) {
    if (t<=0 || t>=1) { memcpy(out,t<=0 ? a : b,12*sizeof(float)); return; }
    Rotation previous,current;
    if (!Decompose(a,previous) || !Decompose(b,current)) {
        // Una matriz colapsada no define rotación; conservar su colapso lineal.
        for (int i=0;i<12;++i) out[i]=RenderBlend(a[i],b[i],t);
        return;
    }
    float dot=0;
    for (int i=0;i<4;++i) dot+=previous.q[i]*current.q[i];
    if (dot<0) { for (float &value:current.q) value=-value; dot=-dot; }
    float wa=1-t,wb=t;
    if (dot<.9995f) {
        const float theta=std::acos(dot), sine=std::sin(theta);
        wa=std::sin((1-t)*theta)/sine; wb=std::sin(t*theta)/sine;
    }
    float q[4],norm=0;
    for (int i=0;i<4;++i) { q[i]=wa*previous.q[i]+wb*current.q[i]; norm+=q[i]*q[i]; }
    norm=std::sqrt(norm);
    for (float &value:q) value/=norm;
    const float x=q[0],y=q[1],z=q[2],w=q[3];
    const float rotation[9]={1-2*(y*y+z*z),2*(x*y-z*w),2*(x*z+y*w),
        2*(x*y+z*w),1-2*(x*x+z*z),2*(y*z-x*w),
        2*(x*z-y*w),2*(y*z+x*w),1-2*(x*x+y*y)};
    for (int row=0;row<3;++row) {
        for (int col=0;col<3;++col) out[row*4+col]=rotation[row*3+col]*RenderBlend(previous.scale[col],current.scale[col],t);
        out[row*4+3]=RenderBlend(a[row*4+3],b[row*4+3],t);
    }
}

bool RenderInverseMatrix(const float *m, float *out) {
    const float determinant=m[0]*(m[5]*m[10]-m[6]*m[9])-m[1]*(m[4]*m[10]-m[6]*m[8])+m[2]*(m[4]*m[9]-m[5]*m[8]);
    if (determinant==0 || !std::isfinite(determinant)) return false;
    const float inv=1/determinant;
    out[0]=(m[5]*m[10]-m[6]*m[9])*inv; out[1]=(m[2]*m[9]-m[1]*m[10])*inv; out[2]=(m[1]*m[6]-m[2]*m[5])*inv;
    out[4]=(m[6]*m[8]-m[4]*m[10])*inv; out[5]=(m[0]*m[10]-m[2]*m[8])*inv; out[6]=(m[2]*m[4]-m[0]*m[6])*inv;
    out[8]=(m[4]*m[9]-m[5]*m[8])*inv; out[9]=(m[1]*m[8]-m[0]*m[9])*inv; out[10]=(m[0]*m[5]-m[1]*m[4])*inv;
    for (int row=0;row<3;++row) out[row*4+3]=-(out[row*4]*m[3]+out[row*4+1]*m[7]+out[row*4+2]*m[11]);
    return true;
}
