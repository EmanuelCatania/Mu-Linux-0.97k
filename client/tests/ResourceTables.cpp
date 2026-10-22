#include "Render/ResourceTables.h"
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
    std::vector<std::size_t> touched;
    // Superar los antiguos cupos y atravesar múltiples fronteras de páginas.
    for(std::size_t request:{std::size_t(1500),std::size_t(3000),std::size_t(6000),std::size_t(17000),std::size_t(32768)}) {
        ResourceTables::EnsureModels(request);ResourceTables::EnsureTextures(request);
        Require(ResourceTables::ModelBase()==models);
        Require(reinterpret_cast<unsigned char*>(ResourceTables::TextureBase())==textures);
        Require(models[0]==0x31 && textures[0]==0x52);
        for(auto slot:touched) Require(models[slot*0xbc]==0x73 && textures[slot*0x38]==0x94);
        const auto mc=ResourceTables::ModelCount(),tc=ResourceTables::TextureCount();
        Require(mc>=request && tc>=request);
        for(auto i=previousModels*0xbc;i<mc*0xbc;++i) Require(models[i]==0);
        for(auto i=previousTextures*0x38;i<tc*0x38;++i) Require(textures[i]==0);
        models[(request-1)*0xbc]=0x73;textures[(request-1)*0x38]=0x94;
        touched.push_back(request-1);previousModels=mc;previousTextures=tc;
    }
    for(std::size_t invalid:{std::size_t(32769),(std::numeric_limits<std::size_t>::max)()}) {
        bool rejected=false;try { ResourceTables::EnsureModels(invalid); } catch(const std::length_error&) { rejected=true; }Require(rejected);
        rejected=false;try { ResourceTables::EnsureTextures(invalid); } catch(const std::length_error&) { rejected=true; }Require(rejected);
    }
    Require(models[0]==0x31 && textures[0]==0x52);
    ResourceTables::ResetModels();Require(ResourceTables::ModelBase()==models);
    for(std::size_t i=0;i<ResourceTables::ModelCount()*0xbc;++i) Require(models[i]==0);
    Require(textures[0]==0x52);
    std::puts("Recursos: crecimiento/ceros/datos/base estable/reset e indices ABI correctos");
}
