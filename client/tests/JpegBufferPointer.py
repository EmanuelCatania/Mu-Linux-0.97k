#!/usr/bin/env python3
"""OpenJpegBuffer y codec reales: OZJ versionado, muestras JPEG y destino alto."""
import argparse
import json
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import ROOT, build_and_run, function, read_source

SERVICES=r'''
#include <string>
#include <utility>
std::string selectedFile,expectedPath;
unsigned opens,closes,errors,dialogs,quits;
namespace Platform {
struct FileSystem{FILE* Open(const char* path,const char* mode){Require(expectedPath==path && std::strcmp(mode,"rb")==0);++opens;return std::fopen(selectedFile.c_str(),mode);}}fs;
FileSystem& GetFileSystem(){return fs;}
struct Window{void ShowError(const char*,const char*,bool close,bool other){Require(close&&!other);++dialogs;}}window;
Window& GetWindow(){return window;}
}
struct Window{void RequestCloseApplication(){++quits;}}gWindow;
void CErrorReport__Write(int address,const char*){Require(address==0x55c9bf0);++errors;}
int TrackedClose(FILE* file){++closes;return std::fclose(file);}
'''
MAIN=r'''
void Load(char* path,float* target){OpenJpegBuffer(path,ARG);}
std::vector<BYTE> Read(const std::string& path){FILE* file=std::fopen(path.c_str(),"rb");if(!file)return {};Require(std::fseek(file,0,SEEK_END)==0);const long size=std::ftell(file);Require(size>=0 && std::fseek(file,0,SEEK_SET)==0);std::vector<BYTE> bytes(size);Require(std::fread(bytes.data(),1,bytes.size(),file)==bytes.size());std::fclose(file);return bytes;}
std::vector<std::string> MakeFiles(){
    std::vector<std::string> files;unsigned index=0;
    for(auto size:std::vector<std::pair<int,int>>{{1,1},{2,3},{3,2},{7,9},{16,17},{31,1},{256,256},{257,1},{1,257}}){
        const auto name="jpeg-"+std::to_string(index++)+".ozj";FILE* file=std::fopen(name.c_str(),"wb");Require(file);BYTE prefix[24]{};Require(std::fwrite(prefix,1,24,file)==24);
        std::vector<BYTE> rgb(size.first*size.second*3);for(unsigned i=0;i<rgb.size();++i)rgb[i]=static_cast<BYTE>(i*37u+index*19u);
        Require(ImageCodec::WriteJpeg(file,size.first,size.second,rgb.data(),95));std::fclose(file);files.push_back(name);
    }
    files.push_back(ASSET);
    FILE* file=std::fopen("invalid.ozj","wb");Require(file);BYTE bad[32]{};Require(std::fwrite(bad,1,32,file)==32);std::fclose(file);files.push_back("invalid.ozj");files.push_back("missing-file-for-b1.ozj");return files;
}
std::uint64_t Run(BYTE* memory,const std::vector<std::string>& files){
    constexpr unsigned bytes=256*256*3*4+32;std::vector<BYTE> expected(bytes);
    std::uint64_t digest=14695981039346656037ull;unsigned cases=0;
    for(unsigned mode=0;mode<2;++mode)for(unsigned index=0;index<files.size();++index){
        selectedFile=files[index];const auto before=Read(selectedFile);FILE* file=std::fopen(selectedFile.c_str(),"rb");ImageCodec::Image decoded;
        const bool exists=file!=nullptr;const bool ok=file&&ImageCodec::Decode(file,24,ImageCodec::Format::Jpeg,256,decoded);if(file)std::fclose(file);
        if(index==9)Require(ok && decoded.width==256 && decoded.height==256 && decoded.channels==3);
        std::memset(memory,0x5a,bytes);std::memcpy(expected.data(),memory,bytes);
        if(ok)for(int row=0;row<decoded.height;++row)for(int column=0;column<decoded.width*3;++column){const float value=static_cast<float>(decoded.pixels[(decoded.height-1-row)*decoded.width*3+column])*_DAT_00552b70;std::memcpy(expected.data()+16+(row*decoded.width*3+column)*4,&value,4);}
        char path[]="World1\\TerrainLight.jpg";char original[sizeof(path)];std::memcpy(original,path,sizeof(path));DAT_0055a7c4=static_cast<char>(mode);
        expectedPath=mode?std::string(DAT_0055a79c)+"World1\\TerrainLight.OZJ":std::string(DAT_0055a7a4)+path;
        opens=closes=errors=dialogs=quits=0;Load(path,reinterpret_cast<float*>(memory+16));
        Require(opens==1 && closes==(exists?1u:0u) && errors==(ok?0u:exists?1u:2u) && dialogs==(exists?0u:1u) && quits==dialogs);
        Require(std::memcmp(memory,expected.data(),bytes)==0 && std::memcmp(path,original,sizeof(path))==0 && Read(selectedFile)==before);
        for(BYTE value:expected){digest^=value;digest*=1099511628211ull;}++cases;
    }Require(cases==24);return digest;
}
int main(){const auto files=MakeFiles();auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low,files);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);std::memset(low,0x7b,256*256*3*4+32);Require(Run(high,files)==expected);for(unsigned i=0;i<256*256*3*4+32;++i)Require(low[i]==0x7b);std::puts("destino RGB alto: bytes/filas/archivos iguales; alias bajo intacto");}
    std::printf("JPEG buffer: 24 archivos/modos, codec real/OZJ/bytes %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    codec='src/Render/Texture/ImageCodec.cpp'
    if revision and read_source(codec,revision)!=read_source(codec):raise ValueError('El codec difiere de la referencia')
    prefix='#define NOMINMAX\n'+PREAMBLE.split('char DAT_00559bb4',1)[0].replace('0x10000','0x100000')
    prefix+='\n#include '+json.dumps((ROOT/codec).as_posix())+'\n'+SERVICES
    header,definitions=read_source('src/globals.h',revision),read_source('src/globals.cpp',revision)
    for name in ['DAT_0055a79c','DAT_0055a7a4','DAT_00561b70','DAT_0055a7c4','_DAT_00552b70']:
        prefix+=re.search(rf'extern [^;\n]*\b{name}(?:\[[^\]]*\])?\s*;',header).group()
        prefix+=re.search(rf'^[^/\n]*\b{name}(?:\[[^\]]*\])?\s*=\s*[^;]+;',definitions,re.M).group()
    code=function(read_source('src/Terrain/Terrain.cpp',revision),'OpenJpegBuffer')
    arg='target' if 'float* dst' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(target))'
    return prefix+'\n#define fclose TrackedClose\n'+code+'\n#undef fclose\n'+MAIN.replace('ARG',arg).replace('ASSET',json.dumps((ROOT/'bin/Client/Data/World1/TerrainLight.OZJ').as_posix()))


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-jpeg-buffer-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
