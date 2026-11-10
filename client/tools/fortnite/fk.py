import math, struct, sys
def qmul(a,b):
    ax,ay,az,aw=a; bx,by,bz,bw=b
    return (aw*bx+ax*bw+ay*bz-az*by, aw*by-ax*bz+ay*bw+az*bx, aw*bz+ax*by-ay*bx+az*bw, aw*bw-ax*bx-ay*by-az*bz)
def qinv(q): return (-q[0],-q[1],-q[2],q[3])
def qrot(q,v):
    x,y,z,w=q; vx,vy,vz=v
    # v' = q v q^-1
    tx=2*(y*vz-z*vy); ty=2*(z*vx-x*vz); tz=2*(x*vy-y*vx)
    return (vx+w*tx+(y*tz-z*ty), vy+w*ty+(z*tx-x*tz), vz+w*tz+(x*ty-y*tx))
def mu_euler_quat(a):
    sz,cz=math.sin(a[2]*.5),math.cos(a[2]*.5); sy,cy=math.sin(a[1]*.5),math.cos(a[1]*.5); sx,cx=math.sin(a[0]*.5),math.cos(a[0]*.5)
    return (sx*cy*cz-cx*sy*sz, sx*cy*sz+cx*sy*cz, cx*cy*sz-sx*sy*cz, cx*cy*cz+sx*sy*sz)
def fk(parents,pos,quat):
    W=[None]*len(parents)
    for i,p in enumerate(parents):
        if p<0: W[i]=(pos[i],quat[i])
        else:
            pp,pq=W[p]; r=qrot(pq,pos[i]); W[i]=((pp[0]+r[0],pp[1]+r[1],pp[2]+r[2]),qmul(pq,quat[i]))
    return W
