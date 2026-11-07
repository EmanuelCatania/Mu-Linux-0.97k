#pragma once
#include <cstddef>
#include <cstdint>

namespace TransformMath {
struct Mesh {
    const unsigned char* vertices;
    const unsigned char* normals;
    float* positions;
    float* rotatedNormals;
    float* light;
    std::size_t vertexCount, normalCount;
};
struct Batch {
    const Mesh* meshes;
    std::size_t meshCount;
    const float* bones;
    float rotationScale, bodyScale, origin[3], direction[3], coefficients[3];
    bool directTransform, translate, lighting;
};
// IDA: VectorRotate 0x004FA0B0 / VectorTransform 0x004FA170.
// Conservar el orden de operaciones y los stores float del original.
inline void Rotate(const float* p, const float* m, float* out) {
    out[0] = p[0]*m[0] + m[2]*p[2] + m[1]*p[1];
    out[1] = m[6]*p[2] + m[5]*p[1] + m[4]*p[0];
    out[2] = m[10]*p[2] + m[9]*p[1] + m[8]*p[0];
}
inline void Run(void* context, std::size_t begin, std::size_t end) {
    const auto& b = *static_cast<const Batch*>(context);
    std::size_t offset = 0;
    for (std::size_t j=0; j<b.meshCount && offset<end; ++j) {
        const auto& mesh=b.meshes[j];
        for (std::size_t i=begin>offset?begin-offset:0;
             i<mesh.vertexCount && offset+i<end; ++i) {
            const auto* raw=mesh.vertices+i*16;
            const auto* p=reinterpret_cast<const float*>(raw+4);
            const auto* m=b.bones+*reinterpret_cast<const std::int16_t*>(raw)*12;
            auto* out=mesh.positions+i*3;
            if (b.directTransform) {
                out[0]=p[0]*m[0]+m[2]*p[2]+m[1]*p[1]+m[3];
                out[1]=m[6]*p[2]+m[5]*p[1]+m[4]*p[0]+m[7];
                out[2]=m[10]*p[2]+m[9]*p[1]+m[8]*p[0]+m[11];
            } else {
                Rotate(p,m,out);
                out[0]=b.rotationScale*out[0]+m[3];
                out[1]=b.rotationScale*out[1]+m[7];
                out[2]=b.rotationScale*out[2]+m[11];
            }
            if(b.translate) {
                out[0]*=b.bodyScale; out[1]*=b.bodyScale; out[2]*=b.bodyScale;
                out[0]+=b.origin[0]; out[1]+=b.origin[1]; out[2]+=b.origin[2];
            }
        }
        offset+=mesh.vertexCount;
        for (std::size_t i=begin>offset?begin-offset:0;
             i<mesh.normalCount && offset+i<end; ++i) {
            const auto* raw=mesh.normals+i*20;
            const auto* m=b.bones+*reinterpret_cast<const std::int16_t*>(raw)*12;
            auto* out=mesh.rotatedNormals+i*3;
            Rotate(reinterpret_cast<const float*>(raw+4),m,out);
            if(b.lighting) {
                float lit=(b.direction[1]*out[1]+b.direction[0]*out[0]+b.direction[2]*out[2])
                    *b.coefficients[0]+b.coefficients[1];
                if(lit<b.coefficients[2]) lit=b.coefficients[2];
                mesh.light[i]=lit;
            }
        }
        offset+=mesh.normalCount;
    }
}
}
