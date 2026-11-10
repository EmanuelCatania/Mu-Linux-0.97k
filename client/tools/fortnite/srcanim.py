import struct, math
from mdl import d, bones, nla, lai, nls, lsi, cs

def half(h):
    return struct.unpack('<e',struct.pack('<H',h))[0]

def euler_quat(a):
    # Source AngleQuaternion: a=[roll(x), pitch(y), yaw(z)] radianes
    sr,cr=math.sin(a[0]*.5),math.cos(a[0]*.5)
    sp,cp=math.sin(a[1]*.5),math.cos(a[1]*.5)
    sy,cy=math.sin(a[2]*.5),math.cos(a[2]*.5)
    return (sr*cp*cy-cr*sp*sy, cr*sp*cy+sr*cp*sy, cr*cp*sy-sr*sp*cy, cr*cp*cy+sr*sp*sy)  # x,y,z,w

def q48(o):
    s0,s1,s2=struct.unpack_from('<3H',d,o)
    x=(s0-32768)/32768.0; y=(s1-32768)/32768.0; z=((s2&0x7fff)-16384)/16384.0
    w=math.sqrt(max(0,1-x*x-y*y-z*z)); 
    if s2&0x8000: w=-w
    return (x,y,z,w)
def q64(o):
    v,=struct.unpack_from('<Q',d,o)
    x=(v&0x1FFFFF); y=(v>>21)&0x1FFFFF; z=(v>>42)&0x1FFFFF; wn=v>>63
    k=1/1048576.5
    x=(x-1048576)*k; y=(y-1048576)*k; z=(z-1048576)*k
    w=math.sqrt(max(0,1-x*x-y*y-z*z)); 
    if wn: w=-w
    return (x,y,z,w)
def extract(o_base, frame, scale):
    # o_base: offset absoluto del primer anim value, 0 -> 0
    if o_base==0: return 0.0
    p=o_base; k=frame
    while True:
        valid,total=struct.unpack_from('<BB',d,p)
        if total>k: break
        k-=total; p+=(valid+1)*2
    valid,total=struct.unpack_from('<BB',d,p)
    if valid>k: v,=struct.unpack_from('<h',d,p+2+2*k)
    else: v,=struct.unpack_from('<h',d,p+2*valid)
    return v*scale

def animdesc(i):
    o=lai+i*100
    nf,=struct.unpack_from('<i',d,o+16); fps,=struct.unpack_from('<f',d,o+8); flags,=struct.unpack_from('<i',d,o+12)
    aidx,=struct.unpack_from('<i',d,o+56); sidx,sfr=struct.unpack_from('<ii',d,o+80)
    return o,nf,fps,flags,aidx,sidx,sfr

def frame_pose(i, frame):
    o,nf,fps,flags,aidx,sidx,sfr=animdesc(i)
    f=frame
    if sfr!=0 and nf>sfr:
        if f==nf-1: f=0; sec=(nf//sfr)+1
        else: sec=f//sfr; f-=sec*sfr
        base=o+struct.unpack_from('<2i',d,o+sidx+sec*8)[1]
    else: base=o+aidx
    pos=[b[2] for b in bones]; quat=[b[3] for b in bones]
    p=base
    while True:
        bone,fl,nxt=struct.unpack_from('<BBh',d,p)
        b=bones[bone]; off=p+4
        q=None; ps=None
        if fl&0x02: q=q48(off); off+=6
        elif fl&0x20: q=q64(off); off+=8
        elif fl&0x08:
            offs=struct.unpack_from('<3h',d,off); a=[b[4][j]+extract(off+offs[j] if offs[j] else 0,f,b[6][j]) for j in range(3)]; q=euler_quat(a); off+=6
        if fl&0x01:
            ps=tuple(half(h) for h in struct.unpack_from('<3H',d,off)); off+=6
        elif fl&0x04:
            offs=struct.unpack_from('<3h',d,off); ps=tuple(b[2][j]+extract(off+offs[j] if offs[j] else 0,f,b[5][j]) for j in range(3)); off+=6
        if q: quat[bone]=q
        if ps: pos[bone]=ps
        if nxt==0: break
        p+=nxt
    return pos,quat
if __name__=='__main__':
    pos,quat=frame_pose(24,10)
    print(pos[0],quat[0]); print([round(sum(c*c for c in q),3) for q in quat[:6]])
