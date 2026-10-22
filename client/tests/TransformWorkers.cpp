#include "Render/TransformWorkers.h"
#include "Render/TransformMath.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>
#include <atomic>

#define CHECK(x) do { if(!(x)) { std::fprintf(stderr,"Fallo linea %d: %s\n",__LINE__,#x); std::exit(1); } } while(0)
#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif
NOINLINE void OriginalRotate(const float* p,const float* m,float* o) {
    *o=*p**m+m[2]*p[2]+m[1]*p[1];
    o[1]=m[6]*p[2]+m[5]*p[1]+m[4]**p;
    o[2]=m[10]*p[2]+m[9]*p[1]+m[8]**p;
}
NOINLINE void OriginalTransform(const float* p,const float* m,float* o) {
    *o=*p**m+m[2]*p[2]+m[1]*p[1]+m[3];
    o[1]=m[6]*p[2]+m[5]*p[1]+m[4]**p+m[7];
    o[2]=m[10]*p[2]+m[9]*p[1]+m[8]**p+m[11];
}
void Reference(TransformMath::Batch& b) {
    for(std::size_t j=0;j<b.meshCount;++j) {
        const auto& mesh=b.meshes[j];
        for(std::size_t i=0;i<mesh.vertexCount;++i) {
            const auto* raw=mesh.vertices+i*16;
            const auto* p=reinterpret_cast<const float*>(raw+4);
            const auto* m=b.bones+*reinterpret_cast<const std::int16_t*>(raw)*12;
            float* o=mesh.positions+i*3;
            if(b.directTransform) OriginalTransform(p,m,o);
            else {
                OriginalRotate(p,m,o);
                o[0]=b.rotationScale*o[0]+m[3];
                o[1]=b.rotationScale*o[1]+m[7];
                o[2]=b.rotationScale*o[2]+m[11];
            }
            if(b.translate) {
                o[0]*=b.bodyScale;o[1]*=b.bodyScale;o[2]*=b.bodyScale;
                o[0]+=b.origin[0];o[1]+=b.origin[1];o[2]+=b.origin[2];
            }
        }
        for(std::size_t i=0;i<mesh.normalCount;++i) {
            const auto* raw=mesh.normals+i*20;
            auto* o=mesh.rotatedNormals+i*3;
            OriginalRotate(reinterpret_cast<const float*>(raw+4),b.bones+*reinterpret_cast<const std::int16_t*>(raw)*12,o);
            if(b.lighting) {
                float lit=(b.direction[1]*o[1]+b.direction[0]*o[0]+b.direction[2]*o[2])*b.coefficients[0]+b.coefficients[1];
                if(lit<b.coefficients[2])lit=b.coefficients[2];
                mesh.light[i]=lit;
            }
        }
    }
}
int main(int argc, char** argv) {
    TransformWorkers::Team team;
    std::mt19937 rng(4404);
    std::uniform_real_distribution<float> random(-12.f,12.f);
    std::vector<float> bones(200*12);
    for(auto& f:bones)f=random(rng);
    struct Storage {
        std::vector<unsigned char> vertices,normals;
        std::vector<float> output,reference;
        std::size_t vc,nc;
    };
    std::vector<Storage> storage(7);
    std::vector<TransformMath::Mesh> meshes;
    std::size_t count=0;
    for(std::size_t j=0;j<storage.size();++j) {
        auto& s=storage[j]; s.vc=j==0?0:123+j*1903; s.nc=j==1?0:151+j*1371;
        s.vertices.resize(s.vc*16);s.normals.resize(s.nc*20);
        for(int normal=0;normal<2;++normal) {
            auto& raw=normal?s.normals:s.vertices;std::size_t stride=normal?20:16;
            for(std::size_t i=0;i<raw.size()/stride;++i) {
                const std::int16_t bone=static_cast<std::int16_t>(rng()%200);
                std::memcpy(raw.data()+i*stride,&bone,2);
                float p[3]{random(rng),random(rng),random(rng)};
                if(i%17==0) { p[0]=1.e-40f; p[1]=-1.e-40f; p[2]=-0.f; }
                std::memcpy(raw.data()+i*stride+4,p,12);
            }
        }
        s.output.resize((s.vc+s.nc)*3+s.nc+16,12345.f);s.reference=s.output;
        meshes.push_back({s.vertices.data(),s.normals.data(),s.output.data(),s.output.data()+s.vc*3,
            s.output.data()+(s.vc+s.nc)*3,s.vc,s.nc});count+=s.vc+s.nc;
    }
    TransformMath::Batch batch{meshes.data(),meshes.size(),bones.data(),1.2f,0.7f,
        {19,-21,33},{0.3f,-1.5f,2},{0.8f,0.4f,0.2f},false,true,true};
    const unsigned savedMxcsr=_mm_getcsr();const int savedRound=std::fegetround();
    unsigned cases=0;
    for(int rounding : {FE_TONEAREST,FE_DOWNWARD,FE_UPWARD,FE_TOWARDZERO}) {
        CHECK(std::fesetround(rounding)==0);
        for(bool denormals : {false,true}) {
            _mm_setcsr((_mm_getcsr()&~0x8040u)|(denormals?0x8040u:0u));
            for(int flags=0;flags<8;++flags) {
                batch.directTransform=(flags&1)!=0;batch.translate=(flags&2)!=0;batch.lighting=(flags&4)!=0;
                for(auto& s:storage)std::fill(s.output.begin(),s.output.end(),12345.f);
                Reference(batch);
                for(auto& s:storage) {s.reference=s.output;std::fill(s.output.begin(),s.output.end(),12345.f);}
                team.Run(count,&batch,TransformMath::Run);
                for(auto& s:storage)CHECK(std::memcmp(s.output.data(),s.reference.data(),s.output.size()*4)==0);
                ++cases;
            }
        }
    }
    std::fesetround(savedRound);_mm_setcsr(savedMxcsr);
    // Miles de barreras, incluido tamaÃ±o cero e intervalos no divisibles.
    std::vector<std::atomic<unsigned>> visits(257);
    struct Coverage {std::atomic<unsigned>* values;}; Coverage coverage{visits.data()};
    auto visit=[](void* p,std::size_t begin,std::size_t end) {
        auto* values=static_cast<Coverage*>(p)->values;
        for(auto i=begin;i<end;++i)++values[i];
    };
    for(unsigned n=0;n<2000;++n) {
        for(auto& value:visits)value=0;
        const auto size=n%258;team.Run(size,&coverage,visit);
        for(unsigned i=0;i<257;++i)CHECK(visits[i]==(i<size?1u:0u));
    }
    batch.directTransform=false;batch.translate=true;batch.lighting=true;
    std::printf("%u casos bit a bit; 2000 barreras; %u trabajadores\n",cases,team.Workers());
    if(argc<2 || std::strcmp(argv[1],"--bench")!=0) return 0;
    auto measure=[&](bool parallel,std::size_t size) {
        const auto start=std::chrono::steady_clock::now();
        for(int i=0;i<1000;++i) {
            if(parallel)team.Run(size,&batch,TransformMath::Run);
            else TransformMath::Run(&batch,0,size);
        }
        return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    };
    const auto referenceStart=std::chrono::steady_clock::now();
    for(int i=0;i<1000;++i) Reference(batch);
    std::printf("Referencia original completa: %.3f ms\n",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-referenceStart).count());
    std::printf("%u casos bit a bit; 2000 barreras; %u trabajadores\n",cases,team.Workers());
    for(std::size_t size : {std::size_t(2048),std::size_t(4096),std::size_t(8192),std::size_t(16384),std::size_t(32768),count}) {
        const double serial=measure(false,size),parallel=measure(true,size);
        std::printf("%zu elementos; serial %.3f ms paralelo %.3f ms (1000 poses)\n",size,serial,parallel);
    }
}
