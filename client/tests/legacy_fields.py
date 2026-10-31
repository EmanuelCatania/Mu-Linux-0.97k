"""Devuelve accesos con nombre a offsets crudos, para bancos x86 con memoria propia."""
import re
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
h=(ROOT/'src/Entity/EntityTypes.h').read_text(encoding='utf-8',errors='replace')
size={'BYTE':1,'char':1,'short':2,'WORD':2,'int':4,'float':4,'vec3_t':12}
def parse(block):
    out=[]
    for m in re.finditer(r'^\s+(\w+)(\*?)\s+(\*?)(\w+)((?:\[[^\]]*\])*);\s*//\s*0x([0-9A-Fa-f]+)',block,re.M):
        out.append((int(m[6],16),m[1],m[2]+m[3],m[4],m[5]))
    return out
ob=parse(h[h.index('struct OBJECT {'):h.index('struct PART_t')])
ch=parse(h[h.index('struct CHARACTER {'):h.index('#pragma pack(pop)')])
tab={}
def add(off,expr,st): tab[off]=(st,expr)
for o,t,p,n,a in ob:
    if n.startswith('_unk') or o<0xFC: continue
    if t=='vec3_t' and not a:
        for i in range(3): add(o+4*i,f'{n}[{i}]','OBJECT')
    elif a: 
        if n=='OBB':
            for i in range(12): add(o+4*i,f'OBB[{i}]','OBJECT')
    else: add(o,n,'OBJECT')
part=[(0,'Type'),(2,'Level'),(3,'Option1'),(4,'LinkBone'),(5,'CurrentAction'),(6,'PriorAction'),(8,'AnimationFrame'),(0xC,'PriorAnimationFrame'),(0x10,'PlaySpeed'),(0x14,'Cloth')]
for o,t,p,n,a in ch:
    if n.startswith('_unk') or n=='Object': continue
    if t=='PART_t':
        c=int(a[1:-1]) if a else 1
        for i in range(c):
            for d,f in part: add(o+0x18*i+d,f'{n}[{i}].{f}' if a else f'{n}.{f}','CHARACTER')
    elif t=='vec3_t':
        for i in range(3): add(o+4*i,f'{n}[{i}]','CHARACTER')
    elif a:
        for i in range(int(a[1:-1])): add(o+i,f'{n}[{i}]','CHARACTER')
    else: add(o,n,'CHARACTER')

reverse={(st,e):o for o,(st,e) in tab.items()}
pattern=re.compile(r'\*\(([^()]*\*)\)&\(\((OBJECT|CHARACTER)\*\)\(uintptr_t\)\((\w+)\)\)->(\w+(?:\[\d+\])?(?:\.\w+)?)')

def lower(source):
    return pattern.sub(lambda m:'*(%s)(%s + %d)'%(m[1],m[3],reverse[(m[2],m[4])]),source)
