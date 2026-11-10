import math, struct, sys, os
sys.path.insert(0,os.path.join(os.path.dirname(os.path.abspath(__file__)),'..'))
from fk import *
import gen_customanim as g
import srcanim as sa
from mdl import bones as sbones

# ---- esqueleto MU (destino) ----
MU_PATH=os.environ['MU_PLAYER_BMD']
A_mu,B_mu=g.load(MU_PATH)
mn=[(b[0].split(b'\0')[0].decode() if b else None) for b in B_mu]
mpar=[(b[1] if b else -1) for b in B_mu]
def mu_bind():
    pos=[];quat=[]
    for b in B_mu:
        if b is None: pos.append((0,0,0)); quat.append((0,0,0,1)); continue
        kp,kr=b[2][0]; pos.append(struct.unpack_from('<3f',kp,0)); quat.append(mu_euler_quat(struct.unpack_from('<3f',kr,0)))
    return pos,quat
mpos,mquat=mu_bind()
mW=fk(mpar,mpos,mquat)
sn=[b[0].replace('ValveBiped.Bip01_','') for b in sbones]
spar=[b[1] for b in sbones]
sbpos=[b[2] for b in sbones]; sbquat=[b[3] for b in sbones]
sW=fk(spar,sbpos,sbquat)
sidx={n:i for i,n in enumerate(sn)}
midx={n:i for i,n in enumerate(mn) if n}

# mapa MU -> Source
MAP={'Bip01':'Pelvis','Bip01 Pelvis':'Pelvis',
 'Bip01 Spine':'Spine','Bip01 Spine1':'Spine2','Bip01 Neck':'Spine4','Bip01 Head':'Head1',
 'Bip01 L Thigh':'L_Thigh','Bip01 L Calf':'L_Calf','Bip01 L Foot':'L_Foot','Bip01 L Toe0':'L_Toe0',
 'Bip01 R Thigh':'R_Thigh','Bip01 R Calf':'R_Calf','Bip01 R Foot':'R_Foot','Bip01 R Toe0':'R_Toe0',
 'Bip01 L Clavicle':'L_Clavicle','Bip01 L UpperArm':'L_UpperArm','Bip01 L Forearm':'L_Forearm','Bip01 L Hand':'L_Hand',
 'Bip01 R Clavicle':'R_Clavicle','Bip01 R UpperArm':'R_UpperArm','Bip01 R Forearm':'R_Forearm','Bip01 R Hand':'R_Hand'}
# huesos por direccion: MU hueso -> MU hueso hijo (y hijo Source)
DIR={'Bip01 L Thigh':('Bip01 L Calf','L_Calf'),'Bip01 L Calf':('Bip01 L Foot','L_Foot'),
     'Bip01 R Thigh':('Bip01 R Calf','R_Calf'),'Bip01 R Calf':('Bip01 R Foot','R_Foot'),
     'Bip01 L UpperArm':('Bip01 L Forearm','L_Forearm'),'Bip01 L Forearm':('Bip01 L Hand','L_Hand'),
     'Bip01 R UpperArm':('Bip01 R Forearm','R_Forearm'),'Bip01 R Forearm':('Bip01 R Hand','R_Hand')}
SCALE=mW[midx['Bip01 Pelvis']][0][2]/sW[sidx['Pelvis']][0][2]
# Amortiguacion: la malla del hombro y de la muneca del MU se deforma si se le copia
# entero el movimiento de Source (que reparte la torsion en huesos auxiliares que el
# MU no tiene). 1.0 = copia completa, 0.0 = pose de reposo.
CLAVICLE_DAMP = 0.5
WRIST_DAMP = 0.0
# Torsion de brazos y piernas: 1.0 = la de la animacion, 0.0 = solo la direccion del hueso.
TWIST = 0.5
IDENT = (0.0, 0.0, 0.0, 1.0)

def norm(v):
    l=math.sqrt(sum(c*c for c in v)); return tuple(c/l for c in v) if l>1e-9 else (0,0,1)
def cross(a,b): return (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
def dot(a,b): return sum(x*y for x,y in zip(a,b))
def swing(a,b):
    a=norm(a);b=norm(b); c=cross(a,b); d=dot(a,b)
    if d<-0.999999:
        ax=cross(a,(1,0,0)) 
        if sum(x*x for x in ax)<1e-6: ax=cross(a,(0,1,0))
        ax=norm(ax); return (ax[0],ax[1],ax[2],0.0)
    q=(c[0],c[1],c[2],1+d); l=math.sqrt(sum(x*x for x in q)); return tuple(x/l for x in q)
def slerp(a,b,t):
    d=sum(x*y for x,y in zip(a,b))
    if d<0: b=tuple(-x for x in b); d=-d
    if d>0.9995: r=tuple(x+(y-x)*t for x,y in zip(a,b))
    else:
        th=math.acos(d); s=math.sin(th); r=tuple((math.sin((1-t)*th)*x+math.sin(t*th)*y)/s for x,y in zip(a,b))
    l=math.sqrt(sum(x*x for x in r)); return tuple(x/l for x in r)

def src_local(seq_anim, f):
    f0=int(math.floor(f)); t=f-f0
    p0,q0=sa.frame_pose(seq_anim,f0)
    if t<1e-6: return p0,q0
    nf=sa.animdesc(seq_anim)[1]; f1=min(f0+1,nf-1)
    p1,q1=sa.frame_pose(seq_anim,f1)
    return [tuple(x+(y-x)*t for x,y in zip(a,b)) for a,b in zip(p0,p1)],[slerp(a,b,t) for a,b in zip(q0,q1)]

def quat_euler(q):
    x,y,z,w=q
    # R = Rz*Ry*Rx
    r20=2*(x*z-w*y); r00=1-2*(y*y+z*z); r10=2*(x*y+w*z); r21=2*(y*z+w*x); r22=1-2*(x*x+y*y)
    pitch=math.asin(max(-1,min(1,-r20)))
    if abs(r20)>0.99999:
        roll=0.0; yaw=math.atan2(-(2*(x*y-w*z)),1-2*(x*x+z*z))
    else:
        yaw=math.atan2(r10,r00); roll=math.atan2(r21,r22)
    return (roll,pitch,yaw)

def heading_fix(spos, squat):
    """Contexto de un emote a partir de su cuadro 0: giro G que deja la pelvis mirando
    al frente del modelo (los emotes de Source miran ~70-90 grados corridos) y posicion
    inicial de la pelvis para que el personaje no se desplace del lugar donde esta."""
    W=fk(spar,spos,squat); j=sidx['Pelvis']
    f=qrot(qmul(W[j][1],qinv(sW[j][1])),(0,-1,0))
    th=math.atan2(f[0],-f[1])
    G=(0.0,0.0,math.sin(-th/2),math.cos(-th/2))
    return {'G':G,'p0':qrot(G,W[j][0])}

def retarget_frame(spos,squat, ctx=None):
    sWa=fk(spar,spos,squat)
    if ctx:
        G=ctx['G']; sWa=[(qrot(G,p),qmul(G,q)) for p,q in sWa]
    n=len(B_mu); Wt=[None]*n; lp=[None]*n; lq=[None]*n
    for i in range(n):
        if B_mu[i] is None:
            Wt[i]=None; lp[i]=(0,0,0); lq[i]=(0,0,0,1); continue
        name=mn[i]; p=mpar[i]
        if name in MAP:
            sj=sidx[MAP[name]]
            if name in DIR:
                # delta del Source + correccion de la pose de reposo: el hueso del MU
                # apunta en reposo en otra direccion que el de Source (brazos ~25 grados).
                # Asi la torsion tambien sale de la animacion y no se retuerce la malla.
                cm,cs_=DIR[name]
                dmb=tuple(a-b for a,b in zip(mW[midx[cm]][0],mW[i][0]))
                dsb=tuple(a-b for a,b in zip(sW[sidx[cs_]][0],sW[sj][0]))
                delta=qmul(sWa[sj][1],qinv(sW[sj][1]))
                wq=qmul(delta,qmul(swing(dmb,dsb),mW[i][1]))
                if TWIST < 1.0:
                    ds_anim=tuple(a-b for a,b in zip(sWa[sidx[cs_]][0],sWa[sj][0]))
                    wq=slerp(qmul(swing(dmb,ds_anim),mW[i][1]),wq,TWIST)
            else:
                delta=qmul(sWa[sj][1],qinv(sW[sj][1]))
                if name.endswith('Clavicle'): delta=slerp(IDENT,delta,CLAVICLE_DAMP)
                wq=qmul(delta,mW[i][1])
                if name.endswith('Hand') and p>=0 and Wt[p] is not None:
                    loc=qmul(qinv(Wt[p][1]),wq); bloc=qmul(qinv(mW[p][1]),mW[i][1])
                    wq=qmul(Wt[p][1],slerp(bloc,loc,WRIST_DAMP))
            # posicion mundo: del bind MU, salvo raiz
            if p<0 or Wt[p] is None: 
                wp=mW[i][0]
            else:
                pw=Wt[p]; r=qrot(pw[1],mpos[i]); wp=(pw[0][0]+r[0],pw[0][1]+r[1],pw[0][2]+r[2])
            if name in ('Bip01','Bip01 Pelvis'):
                sp=sWa[sidx['Pelvis']][0]; b0=sW[sidx['Pelvis']][0]
                if ctx:   # xy relativo al cuadro 0 (no se corre del lugar); z contra el reposo
                    p0=ctx['p0']
                    wp=(mW[i][0][0]+SCALE*(sp[0]-p0[0]), mW[i][0][1]+SCALE*(sp[1]-p0[1]),
                        mW[i][0][2]+SCALE*(sp[2]-b0[2]))
                else:
                    wp=tuple(mW[i][0][k]+SCALE*(sp[k]-b0[k]) for k in range(3))
            Wt[i]=(wp,wq)
            if p<0 or Wt[p] is None:
                lp[i]=wp; lq[i]=wq
            else:
                pw=Wt[p]; lq[i]=qmul(qinv(pw[1]),wq)
                d=tuple(a-b for a,b in zip(wp,pw[0])); lp[i]=qrot(qinv(pw[1]),d)
        else:
            # sin mapeo: conserva el local del bind
            lp[i]=mpos[i]; lq[i]=mquat[i]
            if p<0 or Wt[p] is None: Wt[i]=(mpos[i],mquat[i])
            else:
                pw=Wt[p]; r=qrot(pw[1],mpos[i]); Wt[i]=((pw[0][0]+r[0],pw[0][1]+r[1],pw[0][2]+r[2]),qmul(pw[1],mquat[i]))
    return lp,lq,Wt

FOOT_BONES=[midx[n] for n in ('Bip01 L Foot','Bip01 L Toe0','Bip01 R Foot','Bip01 R Toe0')]

def mu_ground_ref():
    """Altura (z) del punto mas bajo de los pies en la pose de reposo de player.bmd
    (accion 1, cuadro 0): el 'suelo' contra el que se alinean los emotes."""
    pos=[];quat=[]
    for b in B_mu:
        if b is None: pos.append((0,0,0)); quat.append((0,0,0,1)); continue
        kp,kr=b[2][1]; pos.append(struct.unpack_from('<3f',kp,0)); quat.append(mu_euler_quat(struct.unpack_from('<3f',kr,0)))
    W=fk(mpar,pos,quat)
    return min(W[i][0][2] for i in FOOT_BONES)

def foot_min(Wt):
    return min(Wt[i][0][2] for i in FOOT_BONES)

