"""Oráculo de archivo BMD para tests: decodificación independiente y muestras reproducibles."""
from pathlib import Path
import struct
KEY=bytes.fromhex('d17352f6d29acb273eaf593137b3e7a2')
def decode(data):
    if data[3]!=12:return data
    n=struct.unpack_from('<i',data,4)[0];state=0x5e;out=bytearray(data[:4])
    for i,b in enumerate(data[8:8+n]):out.append(((b^KEY[i%16])-state)&255);state=(b+0x3d)&255
    return bytes(out)
def script(name):
    name=name.split(b'\0',1)[0]
    if b'_'not in name:return b''
    letters=name.split(b'_',1)[1].split(b'.',1)[0][:4]
    return bytes(int(c in letters)for c in b'RHSN')if letters and all(c in b'RHSN'for c in letters)else b''
def parse(data):
    data=decode(data);cursor=42
    def take(n):
        nonlocal cursor
        assert n>=0 and cursor+n<=len(data)
        result=data[cursor:cursor+n];cursor+=n;return result
    nm,nb,na=struct.unpack_from('<3h',data,36);meshes=[];actions=[];bones=[]
    for _ in range(nm):
        h=take(10);nv,nn,nt,nf,ti=struct.unpack('<5h',h)
        v=take(nv*16);n=take(nn*20);uv=take(nt*8);faces=b''.join(take(64)[:36]for _ in range(nf));name=take(32)
        meshes.append((h,v,n,uv,faces,name,script(name)))
    for _ in range(na):
        h=take(3);frames,locked=struct.unpack('<hB',h);actions.append((h,take(frames*12)if locked else b''))
    for _ in range(nb):
        flag=take(1);h=b'';keys=[]
        if not flag[0]:
            h=take(34)
            for ah,_ in actions:
                frames=struct.unpack_from('<h',ah)[0];keys.append((take(frames*12),take(frames*12)))
        bones.append((flag,h,keys))
    return data[:42],len(data),cursor,meshes,actions,bones

def fixture(trial,encrypted):
    nm=trial%3;nb=(trial//3)%3;na=(trial//9)%3
    d=bytearray(b'BMD\x0a'+('Fixture%02d'%trial).encode().ljust(32,b'\0')+struct.pack('<3h',nm,nb,na))
    for m in range(nm):
        nv=3 if nb else 0;nn=2 if nb else 0;d+=struct.pack('<5h',nv,nn,3,2,m)
        for v in range(nv):d+=struct.pack('<hh3f',v%nb,0,float(v),float(m),float(v-m)*.25)
        for v in range(nn):d+=struct.pack('<hh3fi',v%nb,0,0.,0.,1.,v)
        d+=struct.pack('<6f',0.,0.,1.,0.,0.,1.)
        for indices in [(0,1,2),(1,0,2)]:
            face=bytearray([0xa5]*64);struct.pack_into('<3h',face,2,*indices);struct.pack_into('<3h',face,26,77,77,77);d+=face
        names=[b'tex_RHSN.tga',b'tex_RZ.tga',b'tex_.tga',b'tex.tga'];d+=names[(trial+m)%4].ljust(32,b'\0')
    frameCounts=[]
    for a in range(na):
        frames=(trial+a)%4;flag=128 if(trial+a)%3 else 0;frameCounts.append(frames);d+=struct.pack('<hB',frames,flag)
        if flag:d+=b''.join(struct.pack('<3f',float(f),.25,-.5)for f in range(frames))
    for b in range(nb):
        flag=255 if(trial+b)%4==0 else 0;d.append(flag)
        if not flag:
            d+=('Bone%d'%b).encode().ljust(32,b'\0')+struct.pack('<h',b-1)
            for a,frames in enumerate(frameCounts):
                d+=b''.join(struct.pack('<3f',float(b+f),float(a),.5)for f in range(frames))
                d+=b''.join(struct.pack('<3f',float(f)*.03125,float(b)*.0625,float(a)*.125)for f in range(frames))
    if not encrypted:return bytes(d)
    state=0x5e;enc=bytearray()
    for i,b in enumerate(d[4:]):x=((b+state)&255)^KEY[i%16];enc.append(x);state=(x+0x3d)&255
    return b'BMD\x0c'+struct.pack('<i',len(enc))+enc

def prepare(root,temp):
    samples=temp/'samples';samples.mkdir();cases=[]
    for trial in range(27):
        for encrypted in [False,True]:cases.append((0,fixture(trial,encrypted)))
    for name in ['Player/Angel.bmd','Player/ArmorClass01.bmd','Logo/Face01.bmd','Object1/Beer03.bmd']:
        cases.append((0,(root/'bin/Client/Data'/name).read_bytes()))
    plain=fixture(0,False)
    cases.extend([(1,None),(1,b'BMD\x0a'),(1,b'BAD'+plain[3:]),(1,plain[:3]+b'\x7f'+plain[4:]),(1,b'BMD\x0c'+bytes(38)),(1,b'BMD\x0c'+struct.pack('<i',0)+bytes(38)),(1,b'BMD\x0c'+struct.pack('<i',9999)+bytes(38)),(2,plain[:36]+struct.pack('<3h',513,0,0)),(2,plain[:36]+struct.pack('<3h',0,-1,0))])
    out=bytearray(struct.pack('<I',len(cases)))
    def blob(value):out.extend(struct.pack('<I',len(value)));out.extend(value)
    for i,(kind,data)in enumerate(cases):
        leaf='%03d.bmd'%i;blob(leaf.encode());out.extend(struct.pack('<I',kind))
        if data is not None:(samples/leaf).write_bytes(data)
        if kind==0:
            header,size,cursor,meshes,actions,bones=parse(data);blob(header);out.extend(struct.pack('<II',size,cursor))
            for mesh in meshes:
                for part in mesh:blob(part)
            for action in actions:
                for part in action:blob(part)
            for flag,h,keys in bones:
                blob(flag);blob(h)
                for pair in keys:
                    for part in pair:blob(part)
        elif kind==2:blob(data[:42])
    (temp/'oracle.bin').write_bytes(out)
    return len(cases)
