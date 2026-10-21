// Sin PCH: gestión de páginas Win32/POSIX sin dependencias del renderer.
#include "Render/ResourceTables.h"
#include "Item/ItemDefines.h"
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif
namespace ResourceTables {
namespace {
class Table {
    char* data=nullptr;
    std::size_t stride,count=0,committed=0,reserved=0,page=0;
    // Los índices BMD/OBJECT son short con signo; preservar su ABI. No hay
    // cupos arbitrarios adicionales (antes 512 modelos /1024 texturas).
    static constexpr std::size_t addressable=std::size_t((std::numeric_limits<short>::max)())+1;
    std::size_t Align(std::size_t value) const { return (value+page-1)/page*page; }
public:
    explicit Table(std::size_t slotStride):stride(slotStride) {
#ifdef _WIN32
        SYSTEM_INFO info;GetSystemInfo(&info);page=info.dwPageSize;
#else
        page=std::size_t(sysconf(_SC_PAGESIZE));
#endif
        reserved=Align(addressable*stride);
#ifdef _WIN32
        data=(char*)VirtualAlloc(nullptr,reserved,MEM_RESERVE,PAGE_NOACCESS);
#else
        data=(char*)mmap(nullptr,reserved,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
        if(data==MAP_FAILED) data=nullptr;
#endif
        if(!data) throw std::bad_alloc();
    }
    ~Table() {
#ifdef _WIN32
        VirtualFree(data,0,MEM_RELEASE);
#else
        munmap(data,reserved);
#endif
    }
    void Ensure(std::size_t requested) {
        if(requested>addressable) throw std::length_error("Indice de recurso no representable en BMD/OBJECT (short)");
        if(requested<=count) return;
        std::size_t next=count?count*2:requested;
        if(next<requested) next=requested;if(next>addressable) next=addressable;
        const std::size_t bytes=Align(next*stride);
        if(bytes>committed) {
#ifdef _WIN32
            if(!VirtualAlloc(data+committed,bytes-committed,MEM_COMMIT,PAGE_READWRITE)) throw std::bad_alloc();
#else
            if(mprotect(data+committed,bytes-committed,PROT_READ|PROT_WRITE)) throw std::bad_alloc();
#endif
            committed=bytes;
        }
        count=next;
    }
    std::size_t Count() const { return count; }
    char* Base() { return data; }
    void Reset() { std::memset(data,0,count*stride); }
};
Table& Models() { static Table table(0xbc);return table; }
Table& Textures() { static Table table(0x38);return table; }
}
void EnsureModels(std::size_t count) { Models().Ensure(count); }
void EnsureTextures(std::size_t count) { Textures().Ensure(count); }
std::size_t ModelCount() { return Models().Count(); }
std::size_t TextureCount() { return Textures().Count(); }
void* ModelBase() { EnsureModels(MODEL_MAX_VANILLA);return Models().Base(); }
char* TextureBase() { EnsureTextures(BITMAP_MAX_VANILLA);return Textures().Base(); }
void ResetModels() { ModelBase();Models().Reset(); }
}
