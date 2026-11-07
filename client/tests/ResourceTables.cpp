#include "Render/ResourceTables.h"
#include "Model/ModelTypes.h"
#include "Core/ResourceLimits.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <limits>
#include <initializer_list>
#include <vector>
#include <stdexcept>
static void Require(bool value) { if(!value) { std::fputs("Fallo en tablas de recursos\n",stderr);std::abort(); } }
int main() {
    auto* models=static_cast<unsigned char*>(ResourceTables::ModelBase());
    auto* textures=reinterpret_cast<unsigned char*>(ResourceTables::TextureBase());
    models[0]=0x31;textures[0]=0x52;
    auto previousModels=ResourceTables::ModelCount(),previousTextures=ResourceTables::TextureCount();
    static_assert(sizeof(BMD)==(sizeof(void*)==4?188:212));
    Require(previousModels==MODEL_MAX_VANILLA && previousTextures==BITMAP_MAX_VANILLA);
    const auto sentinel=reinterpret_cast<std::uint8_t*>(uintptr_t(sizeof(void*)==8?0x123456780ull:0x23456780u));
    const auto secondSentinel=reinterpret_cast<std::uint8_t*>(uintptr_t(sentinel)+16);
    auto* first=reinterpret_cast<BMD*>(models);first->Meshs=sentinel;first->Bones=secondSentinel;
    std::vector<std::size_t> touched;
    // Superar los antiguos cupos y atravesar múltiples fronteras de páginas.
    for(std::size_t request:{std::size_t(1500),std::size_t(3000),std::size_t(6000),std::size_t(17000),std::size_t(32768)}) {
        ResourceTables::EnsureModels(request);ResourceTables::EnsureTextures(request);
        Require(ResourceTables::ModelBase()==models);
        Require(reinterpret_cast<unsigned char*>(ResourceTables::TextureBase())==textures);
        Require(models[0]==0x31 && textures[0]==0x52);
        Require(first->Meshs==sentinel && first->Bones==secondSentinel);
        for(auto slot:touched) Require(models[slot*sizeof(BMD)]==0x73 && textures[slot*0x38]==0x94);
        const auto mc=ResourceTables::ModelCount(),tc=ResourceTables::TextureCount();
        Require(mc>=request && tc>=request);
        for(auto i=previousModels*sizeof(BMD);i<mc*sizeof(BMD);++i) Require(models[i]==0);
        for(auto i=previousTextures*0x38;i<tc*0x38;++i) Require(textures[i]==0);
        models[(request-1)*sizeof(BMD)]=0x73;textures[(request-1)*0x38]=0x94;
        touched.push_back(request-1);previousModels=mc;previousTextures=tc;
    }
    for(std::size_t invalid:{std::size_t(32769),(std::numeric_limits<std::size_t>::max)()}) {
        bool rejected=false;try { ResourceTables::EnsureModels(invalid); } catch(const std::length_error&) { rejected=true; }Require(rejected);
        rejected=false;try { ResourceTables::EnsureTextures(invalid); } catch(const std::length_error&) { rejected=true; }Require(rejected);
    }
    Require(models[0]==0x31 && textures[0]==0x52);
    auto* last=reinterpret_cast<BMD*>(models)+32767;last->Bones=sentinel;
    last->Sounds[4]=0x1234;last->_unkB4[7]=0x5a;
    const auto mc=ResourceTables::ModelCount(),tc=ResourceTables::TextureCount();
    ResourceTables::EnsureModels(0);ResourceTables::EnsureModels(1);
    Require(ResourceTables::ModelCount()==mc && ResourceTables::TextureCount()==tc);
    Require(last->Bones==sentinel && last->Sounds[4]==0x1234 && last->_unkB4[7]==0x5a);
    const std::vector<unsigned char> textureBefore(textures,textures+tc*0x38);
    ResourceTables::ResetModels();Require(ResourceTables::ModelBase()==models);
    for(std::size_t i=0;i<ResourceTables::ModelCount()*sizeof(BMD);++i) Require(models[i]==0);
    Require(std::memcmp(textures,textureBefore.data(),textureBefore.size())==0);
    std::puts("Recursos: crecimiento/ceros/datos/base estable/reset e indices ABI correctos");
}
