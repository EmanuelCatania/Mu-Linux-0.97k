#!/usr/bin/env python3
"""Cuatro cuerpos de socket reales con transporte/cifrado/cola controlados.

Preserva offsets del encabezado heredado; no certifica su layout ni handles x64.
"""
import argparse
import os
from pathlib import Path
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
#include <algorithm>
#include <cstdarg>
#include <string>
#ifndef _WIN32
#define __fastcall
#endif
using uint=unsigned;
#define SOCKET std::uintptr_t
unsigned currentTrial;
#define Require(...) do{if(!(__VA_ARGS__)){std::fprintf(stderr,"socket assertion line %d trial %u\n",__LINE__,currentTrial);std::exit(1);}}while(false)
BYTE* activeContext;
std::uintptr_t expectedSocket;
std::vector<int> sendScript;
std::vector<BYTE> payload,incoming;
std::vector<std::vector<BYTE>> packets;
unsigned sendCalls,closeCalls,resetCalls,receiveCalls,decryptCalls,cleanupCalls,errorCalls,windowCalls,logOnCalls,flushCalls;
unsigned sentTotal,priorReceive;
unsigned handleChanges;
int lastError,startupResult;
std::uint16_t startupVersion;
DWORD SocketClientIsGame;
std::uintptr_t SocketClientSocket;
struct Network {void ResetSessionData(){++resetCalls;}} gNetwork;
namespace Platform {
constexpr int SocketWouldBlock=10035;
constexpr std::uintptr_t InvalidSocket=static_cast<std::uintptr_t>(-1);
struct Socket {
 bool IsOpen(std::uintptr_t socket){return socket!=InvalidSocket;}
 int LastError(){return lastError;}
 int Startup(std::uint16_t requested,std::uint16_t& actual){Require(requested==0x202);actual=startupVersion;return startupResult;}
 int Cleanup(){++cleanupCalls;return 0;}
 int Close(std::uintptr_t socket){Require(socket==expectedSocket);++closeCalls;return 0;}
 int Receive(std::uintptr_t socket,char* out,int capacity,int flags){
    Require(socket==expectedSocket && reinterpret_cast<BYTE*>(out)==activeContext+0x2010+priorReceive && capacity==8192-static_cast<int>(priorReceive) && flags==0);++receiveCalls;
    if(incoming.empty())return startupResult;
    Require(incoming.size()<=static_cast<unsigned>(capacity));std::memcpy(out,incoming.data(),incoming.size());return static_cast<int>(incoming.size());
 }
} socket;
Socket& GetSocket(){return socket;}
struct Window {void ShowError(const char*,const char*,bool a,bool b){Require(!a&&!b);++windowCalls;}} window;
Window& GetWindow(){return window;}
template<std::size_t N,class...Args>int TextFormat(char(&out)[N],const char* format,Args...args){return std::snprintf(out,N,format,args...);}
template<std::size_t N>void TextConcat(char(&out)[N],std::size_t capacity,const char* tail){Require(capacity==N);std::strncat(out,tail,N-std::strlen(out)-1);}
}
int ControlledSend(std::uintptr_t socket,const char* data,int size,int flags){
    // El handle nativo de 8 bytes pisa el inicio del buffer en el layout x86.
    // Se observa esa limitación de B2 sin cambiar ni certificar el layout.
    std::uintptr_t stored;std::memcpy(&stored,activeContext+8,sizeof(stored));
    if(sendCalls && stored!=expectedSocket)++handleChanges;
    expectedSocket=stored;
    Require(socket==expectedSocket && reinterpret_cast<const BYTE*>(data)==activeContext+12 && size==static_cast<int>(payload.size()-sentTotal) && flags==0);
    Require(std::memcmp(data,payload.data()+sentTotal,size)==0 && sendCalls<sendScript.size());
    const int result=sendScript[sendCalls++];Require(result<=size);if(result>0)sentTotal+=result;return result;
}
#define send ControlledSend
int DAT_055c9bf0;
void CErrorReport_Write(int*,const char*){++errorCalls;}
void DbgLogPublic(const char*){}
void CWsctlc__LogPrintOn(){++logOnCalls;}
void FUN_0043de60(){++flushCalls;}
namespace MuEmu {void DecryptRecv(BYTE* ptr,int size){Require(ptr==activeContext+0x2010+priorReceive && size==static_cast<int>(incoming.size()));++decryptCalls;}}
void CPacketQueue_PushPacket(std::uintptr_t context,const void* data,int size){
    Require(context==reinterpret_cast<std::uintptr_t>(activeContext) && static_cast<const BYTE*>(data)>=activeContext+0x2010 && static_cast<const BYTE*>(data)+size<=activeContext+0x4010);
    packets.emplace_back(static_cast<const BYTE*>(data),static_cast<const BYTE*>(data)+size);
}
'''
MAIN=r'''
template<class T>void Put(BYTE* p,T value){std::memcpy(p,&value,sizeof(value));}
template<class T>T Get(const BYTE* p){T value;std::memcpy(&value,p,sizeof(value));return value;}
void Startup(BYTE* context){CWsctlc_Startup(ARG);}
void Close(BYTE* context){CWsctlc_Close(ARG);}
int Disconnect(BYTE* context){return Net_Disconnect(DISCONNECT_ARG);}
int Send(BYTE* context){return CWsctlc_FDWriteSend(ARG);}
void Reset(BYTE* memory){
    std::memset(memory,0x5a,0x5040);activeContext=memory+16;
    sendCalls=closeCalls=resetCalls=receiveCalls=decryptCalls=cleanupCalls=errorCalls=windowCalls=logOnCalls=flushCalls=sentTotal=0;
    sendScript.clear();incoming.clear();payload.clear();packets.clear();SocketClientIsGame=123;handleChanges=0;
}
void Hash(std::uint64_t& digest,const void* bytes,std::size_t size){const BYTE* p=static_cast<const BYTE*>(bytes);while(size--){digest^=*p++;digest*=1099511628211ull;}}
std::uint64_t Run(BYTE* memory){
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<1024;++trial){currentTrial=trial;
        Reset(memory);BYTE expected[0x5040];std::memcpy(expected,memory,sizeof(expected));
        startupResult=trial%4==0?1:0;startupVersion=trial%4==1?0x101:0x202;
        Startup(activeContext);
        if(startupResult){Require(errorCalls==1 && windowCalls==1 && cleanupCalls==0 && logOnCalls==0);}
        else if(startupVersion!=0x202){Require(errorCalls==1 && windowCalls==1 && cleanupCalls==1 && logOnCalls==0);}
        else {Put(expected+16+8,0u);Put(expected+16+4,0x202u);Require(logOnCalls==1 && errorCalls==0 && windowCalls==0 && cleanupCalls==0);}
        Require(std::memcmp(expected,memory,sizeof(expected))==0);Hash(digest,memory,sizeof(expected));
        Reset(memory);const unsigned mode=trial%8;const int length=32+trial%128;
        for(int i=0;i<length;++i)payload.push_back(static_cast<BYTE>(trial*37+i*19));
        std::memcpy(activeContext+12,payload.data(),payload.size());Put(activeContext+0x200c,mode==0?0:mode==1?-3:length);Put(activeContext+0x4014,trial%2?1:0);
        expectedSocket=Get<std::uintptr_t>(activeContext+8);lastError=mode==3?Platform::SocketWouldBlock:10054;
        if(mode==2)sendScript={length};if(mode==3)sendScript={7,-1};if(mode==4)sendScript={-1};if(mode==5)sendScript={0};if(mode==6)sendScript={7,0};if(mode==7)sendScript={7,5,length-12};
        const int result=Send(activeContext);const bool fatal=mode>=4 && mode<=6;
        Require(result==(fatal?0:1) && closeCalls==(fatal?1u:0u) && resetCalls==closeCalls);
        Require(sendCalls==sendScript.size() && Get<int>(activeContext+0x200c)==(mode==0?0:mode==1?-3:length-static_cast<int>(sentTotal)));
        Require(handleChanges==(sizeof(std::uintptr_t)>4?(mode==3 || mode==6?1u:mode==7?2u:0u):0u));
        Require(flushCalls==(trial%2?(mode==2?1u:mode==3||mode==6?1u:mode==7?3u:0u):0u));
        if(fatal)Require(Get<int>(activeContext+8)==-1 && SocketClientIsGame==0);
        Hash(digest,&result,4);Hash(digest,memory,0x5040);
        Reset(memory);priorReceive=0;startupResult=0;const unsigned receiveMode=trial%12;
        std::vector<BYTE> prefix;std::vector<std::vector<BYTE>> wanted;unsigned remain=0;int expectedResult=0;
        if(receiveMode==0){startupResult=0;expectedResult=1;}
        if(receiveMode==1 || receiveMode==2){startupResult=-1;lastError=receiveMode==1?Platform::SocketWouldBlock:10054;expectedResult=1;}
        if(receiveMode==3){incoming={0xc1,5,1,2,3};wanted={incoming};}
        if(receiveMode==4){incoming={0xc2,0,8,1,2,3,4,5};wanted={incoming};}
        if(receiveMode==5){incoming={0xc1,4,1,2,0xc3,5,3,4,5};wanted={{0xc1,4,1,2},{0xc3,5,3,4,5}};}
        if(receiveMode==6){incoming={0xc1,7};remain=2;expectedResult=3;}
        if(receiveMode==7){prefix={0xc1,8};incoming={1,2,3};remain=5;}
        if(receiveMode==8){prefix={0xc1,5,1,2};incoming={3,0xc2,0,10};wanted={{0xc1,5,1,2,3}};remain=3;}
        if(receiveMode==9){incoming={0xaa,1,2};}
        if(receiveMode==10){incoming={0xc1,0,1};remain=3;}
        if(receiveMode==11){incoming={0xc4,0,7,1,2,3,4};wanted={incoming};}
        priorReceive=static_cast<unsigned>(prefix.size());if(priorReceive)std::memcpy(activeContext+0x2010,prefix.data(),prefix.size());Put(activeContext+0x4010,static_cast<int>(priorReceive));Put(activeContext+0x4014,trial%2?1:0);
        expectedSocket=Get<std::uintptr_t>(activeContext+8);const int received=CWsctlc_nRecv(activeContext);
        Require(received==expectedResult && receiveCalls==1 && decryptCalls==(incoming.empty()?0u:1u) && packets==wanted);
        Require(Get<int>(activeContext+0x4010)==static_cast<int>(remain));
        if(remain){std::vector<BYTE> stream=prefix;stream.insert(stream.end(),incoming.begin(),incoming.end());const auto offset=receiveMode==8?5u:0u;Require(std::memcmp(activeContext+0x2010,stream.data()+offset,remain)==0);}
        Hash(digest,&received,4);Hash(digest,memory,0x5040);
        for(const auto& packet:packets)Hash(digest,packet.data(),packet.size());
        Reset(memory);expectedSocket=Get<std::uintptr_t>(activeContext+8);std::memcpy(expected,memory,sizeof(expected));Put(expected+16+8,-1);
        Close(activeContext);Require(closeCalls==1 && resetCalls==1 && SocketClientIsGame==0 && std::memcmp(expected,memory,sizeof(expected))==0);
        Hash(digest,memory,sizeof(expected));
        for(unsigned open=0;open<2;++open){
            Reset(memory);SocketClientSocket=open?0x1234:Platform::InvalidSocket;expectedSocket=SocketClientSocket;
            std::memcpy(expected,memory,sizeof(expected));Require(Disconnect(activeContext)==0);
            Require(closeCalls==open && resetCalls==0 && SocketClientSocket==(open?static_cast<std::uintptr_t>(static_cast<DWORD>(Platform::InvalidSocket)):Platform::InvalidSocket));
            Require(std::memcmp(expected,memory,sizeof(expected))==0);Hash(digest,&closeCalls,4);
        }
    }return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);std::memset(low,0x7b,0x5040);Require(Run(high)==expected);for(unsigned i=0;i<0x5040;++i)Require(low[i]==0x7b);std::puts("contexto sobre 4 GiB, direcciones/colas/eventos completos; alias bajo intacto");}
    std::printf("Socket context: 1024 sesiones controladas, bytes/estados %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    close=function(read_source('src/Net/Crypto.cpp',revision),'CWsctlc_Close')
    startup=function(read_source('src/Render/SMD_Parser.cpp',revision),'CWsctlc_Startup')
    scene=read_source('src/Scene/Scene_CharSelect_Nav.cpp',revision)
    send=function(scene,'CWsctlc_FDWriteSend');recv=function(scene,'CWsctlc_nRecv')
    disconnect=function(read_source('src/Net/Net_Disconnect.cpp',revision),'Net_Disconnect')
    arg='reinterpret_cast<uintptr_t>(context)' if 'uintptr_t ctx' in close else 'static_cast<int>(reinterpret_cast<uintptr_t>(context))'
    disconnect_arg='context' if 'void* ctx' in disconnect else 'static_cast<int>(reinterpret_cast<uintptr_t>(context))'
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+close+startup+send+recv+disconnect+MAIN.replace('DISCONNECT_ARG',disconnect_arg).replace('ARG',arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-socket-context-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
