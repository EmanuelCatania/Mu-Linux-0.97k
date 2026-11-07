#include "Game/FramePointerSet.h"
#include <unordered_set>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <chrono>
int main(){FramePointerSet set;std::unordered_set<const void*> reference;std::uintptr_t seed=37;
 for(int frame=0;frame<30;++frame){set.Reset();reference.clear();if(set.Insert(nullptr))return 1;for(int i=0;i<20000;++i){seed=seed*1664525u+1013904223u;auto value=(seed%10000+1)*16;auto key=reinterpret_cast<const void*>(value);if(set.Insert(key)!=reference.insert(key).second){puts("diferencia");return 1;}if(set.Insert(key)){puts("duplicado");return 1;}}}
 puts("600000 inserciones aleatorias y duplicados: equivalencia correcta tras reset/crecimiento");
}
