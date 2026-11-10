import struct,sys
import os
d=open(os.environ['FORTNITE_MDL'],'rb').read()
def cs(o):
    e=d.index(b'\0',o); return d[o:e].decode('latin1')
idst,ver=struct.unpack_from('<4si',d,0); assert idst==b'IDST' and ver==49, 'se espera un .mdl de Source v49 (CS:GO)'
(nb,bi,nbc,bci,nhs,hsi,nla,lai,nls,lsi)=struct.unpack_from('<10i',d,156)

bones=[]
for i in range(nb):
    o=bi+i*216
    nm,par=struct.unpack_from('<ii',d,o)
    pos=struct.unpack_from('<3f',d,o+32); q=struct.unpack_from('<4f',d,o+44); rot=struct.unpack_from('<3f',d,o+60)
    ps=struct.unpack_from('<3f',d,o+72); rs=struct.unpack_from('<3f',d,o+84); flags=struct.unpack_from('<i',d,o+160)[0]
    bones.append((cs(o+nm),par,pos,q,rot,ps,rs,flags))
if __name__=='__main__':
    for i,b in enumerate(bones): print(i,b[0],b[1])
    seqs=[]
    for i in range(nls):
        o=lsi+i*212
        lab,=struct.unpack_from('<i',d,o+4); print(i,cs(o+lab), struct.unpack_from('<2i',d,o+56)[0], end=' | ')
