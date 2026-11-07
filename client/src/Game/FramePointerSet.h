#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <utility>

// DESVIACION: deduplicar direcciones por cuadro sin asignar un nodo por objeto.
// Se conserva la capacidad entre cuadros; crece cuando la carga lo necesita.
class FramePointerSet {
    std::vector<const void*> slots;
    std::size_t used=0;
    std::size_t Index(const void* key) const {
        auto value=reinterpret_cast<std::uintptr_t>(key);
        value^=value>>4;value*=0x9e3779b9u;value^=value>>16;
        return static_cast<std::size_t>(value)&(slots.size()-1);
    }
    void Grow() {
        auto previous=std::move(slots);
        slots.assign(previous.empty()?256:previous.size()*2,nullptr);used=0;
        for(auto key:previous) if(key) Insert(key);
    }
public:
    void Reset() { std::fill(slots.begin(),slots.end(),nullptr);used=0; }
    bool Insert(const void* key) {
        if(!key) return false;
        if((used+1)*2>=slots.size()) Grow();
        auto index=Index(key);
        while(slots[index]) {
            if(slots[index]==key) return false;
            index=(index+1)&(slots.size()-1);
        }
        slots[index]=key;++used;return true;
    }
};
