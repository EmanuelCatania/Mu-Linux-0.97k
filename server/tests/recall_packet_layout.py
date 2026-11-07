"""Comprueba los siete bytes de C3:B0 contra la declaración real del servidor."""
import argparse
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--architecture', default='Win32', choices=['Win32', 'x64'])
args = parser.parse_args()
header = (root/'Source/MuServer/GameServer/SkillManager.h').read_text(encoding='utf-8')
start = header.index('struct PMSG_SKILL_TELEPORT_ALLY_RECV')
if header[:start].rstrip().endswith('#pragma pack(push, 1)'):
    start = header.rfind('#pragma pack(push, 1)', 0, start)
end = header.index('};', start)+2
if header[end:].lstrip().startswith('#pragma pack(pop)'):
    end = header.index('#pragma pack(pop)', end)+len('#pragma pack(pop)')
code = r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
using BYTE=std::uint8_t; using WORD=std::uint16_t;
struct PBMSG_HEAD { BYTE type,size,head; };
DECLARATION
static_assert(sizeof(PMSG_SKILL_TELEPORT_ALLY_RECV)==7);
static_assert(offsetof(PMSG_SKILL_TELEPORT_ALLY_RECV,index)==3);
static_assert(offsetof(PMSG_SKILL_TELEPORT_ALLY_RECV,x)==5);
static_assert(offsetof(PMSG_SKILL_TELEPORT_ALLY_RECV,y)==6);
struct After { BYTE b; WORD w; };
static_assert(offsetof(After,w)==2);
int main() {
 for(unsigned key=0;key<65536;++key) {
  BYTE wire[7]={0xC3,7,0xB0,BYTE(key),BYTE(key>>8),BYTE(key^0x55),BYTE((key>>8)^0xAA)};
  PMSG_SKILL_TELEPORT_ALLY_RECV request;
  std::memcpy(&request,wire,7);
  if(request.index!=key || request.x!=wire[5] || request.y!=wire[6])return 1;
 }
 std::puts("PASS: 65536 índices LE16, coordenadas y packing posterior de C3:B0");
}
'''.replace('DECLARATION', header[start:end])
with tempfile.TemporaryDirectory(prefix='mu-recall-layout-') as temp:
    path=Path(temp); source=path/'main.cpp'; source.write_text(code,encoding='utf-8')
    if os.name=='nt':
        (path/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.21)\nproject(RecallLayout LANGUAGES CXX)\nadd_executable(recall_test main.cpp)\ntarget_compile_features(recall_test PRIVATE cxx_std_17)\n',encoding='utf-8')
        subprocess.run(['cmake','-S',temp,'-B',str(path/'build'),'-G','Visual Studio 17 2022','-A',args.architecture],check=True)
        subprocess.run(['cmake','--build',str(path/'build'),'--config','Release'],check=True)
        binary=path/'build/Release/recall_test.exe'
    else:
        binary=path/'recall_test'
        subprocess.run([os.environ.get('CXX','c++'),'-std=c++17',str(source),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
