"""Devuelve accesos con nombre a offsets crudos, para bancos x86 con memoria propia."""
import re
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
h=(ROOT/'src/game/entity/EntityTypes.h').read_text(encoding='utf-8',errors='replace')
size={'BYTE':1,'char':1,'short':2,'WORD':2,'int':4,'float':4,'vec3_t':12}
def parse(block):
    out=[]
    for m in re.finditer(r'^\s+(\w+)(\*?)\s+(\*?)(\w+)((?:\[[^\]]*\])*);\s*//\s*0x([0-9A-Fa-f]+)',block,re.M):
        out.append((int(m[6],16),m[1],m[2]+m[3],m[4],m[5]))
    return out
ob=parse(h[h.index('struct OBJECT {'):h.index('struct PART_t')])
ch=parse(h[h.index('struct CHARACTER {'):h.index('#pragma pack(pop)')])
tab={}
ftype={}
def add(off,expr,st,ty=None):
    tab[off]=(st,expr)
    if ty: ftype[(st,expr)]=ty
for o,t,p,n,a in ob:
    if n.startswith('_unk') or o<0xFC: continue
    if t=='vec3_t' and not a:
        for i in range(3): add(o+4*i,f'{n}[{i}]','OBJECT','float')
    elif a: 
        if n=='OBB':
            for i in range(12): add(o+4*i,f'OBB[{i}]','OBJECT','float')
    else: add(o,n,'OBJECT',None if p else t)
part=[(0,'Type'),(2,'Level'),(3,'Option1'),(4,'LinkBone'),(5,'CurrentAction'),(6,'PriorAction'),(8,'AnimationFrame'),(0xC,'PriorAnimationFrame'),(0x10,'PlaySpeed'),(0x14,'Cloth')]
for o,t,p,n,a in ch:
    if n.startswith('_unk') or n=='Object': continue
    if t=='PART_t':
        c=int(a[1:-1]) if a else 1
        for i in range(c):
            for d,f in part: add(o+0x18*i+d,f'{n}[{i}].{f}' if a else f'{n}.{f}','CHARACTER')
    elif t=='vec3_t':
        for i in range(3): add(o+4*i,f'{n}[{i}]','CHARACTER','float')
    elif a:
        for i in range(int(a[1:-1])): add(o+i,f'{n}[{i}]','CHARACTER')
    else: add(o,n,'CHARACTER',None if p else t)


def unk(block,st):
    for m in re.finditer(r'BYTE\s+(_unk\w+)(?:\[(0x[0-9A-Fa-f]+|\d+)\])?;',block):
        o=int(m[1][4:],16); c=int(m[2],0) if m[2] else 1
        for i in range(c):
            if o+i not in tab and o+i>=0xFC: tab[o+i]=(st,f'{m[1]}[{i}]' if m[2] else m[1])
unk(h[h.index('struct OBJECT {'):h.index('struct PART_t')],'OBJECT')
unk(h[h.index('struct CHARACTER {'):h.index('#pragma pack(pop)')],'CHARACTER')
reverse={(st,e):o for o,(st,e) in tab.items()}
pattern=re.compile(r'\*\(([^()]*\*)\)&\(\((OBJECT|CHARACTER)\*\)\(uintptr_t\)\((\w+)\)\)->(\w+(?:\[\d+\])?(?:\.\w+)?)')

# Acceso directo a un campo escalar: ((OBJECT*)(uintptr_t)(o))->Alpha = 1.0f;
direct=re.compile(r'(?<![&\w])\(\((OBJECT|CHARACTER)\*\)\(uintptr_t\)\((\w+)\)\)->(\w+(?:\[\d+\])?)(?![\w.\[])')
SCALARS={'float','int','short','WORD','BYTE','char','DWORD'}

def lower(source):
    source=pattern.sub(lambda m:'*(%s)(%s + %d)'%(m[1],m[3],reverse[(m[2],m[4])]),source)
    def d(m):
        key=(m[1],m[3]); ty=ftype.get(key)
        if key not in reverse or ty not in SCALARS: return m.group(0)
        return '(*(%s*)(%s + %d))'%(ty,m[2],reverse[key])
    return direct.sub(d,source)


# AsObject(x)->Campo / AsCharacter(x)->Campo -> offset x86 crudo, para los harnesses
# que trabajan con memoria en el layout x86 y OBJECT/CHARACTER sin definir. No se
# aplica en read_source: los tests que compilan los structs reales no lo necesitan.
_named = re.compile(r'(&?)\b(AsObject|AsCharacter)\((\w+(?:\[\w+\])?)\)->(\w+(?:\[\d+\])?(?:\.\w+(?:\[\d+\])?)?)(?![\w.\[])')
_size = {'BYTE': 1, 'char': 1, 'short': 2, 'WORD': 2, 'int': 4, 'float': 4, 'DWORD': 4}


def _full(fields):
    """offset/tipo de cada campo de un struct, incluidos los vec3_t y arreglos."""
    out = {}
    for o, t, p, n, a in fields:
        if n.startswith('_unk') or p:
            continue
        if t == 'vec3_t' and not a:
            for i in range(3):
                out[f'{n}[{i}]'] = (o + 4 * i, 'float')
        elif a and a[1:-1].isdigit() and t in _size:
            for i in range(int(a[1:-1])):
                out[f'{n}[{i}]'] = (o + _size[t] * i, t)
        elif not a:
            out[n] = (o, t)
    return out


_objects = _full(ob)
_characters = _full(ch)
_parts = _full(parse(h[h.index('struct PART_t'):h.index('struct CHARACTER {')]))
for o, t, p, n, a in ch:
    if t == 'PART_t':
        count = int(a[1:-1]) if a else 1
        for i in range(count):
            for f, (rel, ty) in _parts.items():
                _characters[(f'{n}[{i}].{f}' if a else f'{n}.{f}')] = (o + 0x18 * i + rel, ty)


def lower_names(source):
    def r(m):
        amp, fn, base, field = m.groups()
        table = _objects if fn == 'AsObject' else _characters
        hit = table.get(field)
        if hit is None and fn == 'AsCharacter' and field.startswith('Object.'):
            hit = _objects.get(field[len('Object.'):])
        if hit is None:
            return m.group(0)
        off, ty = hit
        if amp:
            return '((char*)(%s) + %d)' % (base, off)
        if ty not in SCALARS:
            return m.group(0)
        return '(*(%s*)((char*)(%s) + %d))' % (ty, base, off)
    return _named.sub(r, source)
