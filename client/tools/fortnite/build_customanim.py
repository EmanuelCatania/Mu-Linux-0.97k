#!/usr/bin/env python3
"""Convierte emotes del modelo de Source (CS:GO, .mdl v49) a CustomAnim.bmd.

Los emotes se retargetean del esqueleto ValveBiped al Biped de player.bmd y se
re-muestrean a --fps cuadros por segundo; la velocidad de reproduccion de cada
accion se ajusta para que dure lo mismo que el original.

Uso (variables de entorno por las rutas):
  FORTNITE_MDL=<archivo.mdl> MU_PLAYER_BMD=<player.bmd> \
    build_customanim.py <salida.bmd> <indice|nombre,indice|nombre,...> [--fps 20]
Escribe <salida.bmd> y <salida>.txt (una linea por accion: nombre loop velocidad).
"""
import math, os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import retarget as R
import srcanim as sa
from mdl import nls, lsi, cs

DEFAULTS = (R.CLAVICLE_DAMP, R.WRIST_DAMP, R.TWIST)
PLUGIN_PLAYBACK_RATE = 0.84

def sequences():
    out = []
    for i in range(nls):
        o = lsi + i * 212
        lab, = struct.unpack_from('<i', sa.d, o + 4)
        aii, = struct.unpack_from('<i', sa.d, o + 60)
        a0, = struct.unpack_from('<h', sa.d, o + aii)
        out.append((cs(o + lab), a0))
    return out

def load_sounds(sp_path):
    """Tabla emote -> sonidos desde el plugin de SourceMod (fnemotes_near.sp). Cada fila
    es { nombre, secuencia, secuencia_siguiente, "sonido1,sonido2" }; el sonido de una
    intro vale tambien para su loop."""
    import re
    out = {}
    row = re.compile(r'\{\s*"([^"]*)",\s*"([^"]*)",\s*"([^"]*)",\s*"([^"]*)"')
    for line in open(sp_path, encoding='utf-8', errors='replace'):
        m = row.search(line)
        if not m: continue
        name, _seq, nxt, snd = m.groups()
        if not snd: continue
        out[name] = snd
        if nxt and nxt != 'none': out[nxt] = snd
    return out

# Reemplazos propios: el baile clasico usa siempre ninja_dance_01.
SOUND_OVERRIDE = {'DanceMoves': 'ninja_dance_01'}

def convert(anim, fps_out):
    o, nf, fps, flags, *_ = sa.animdesc(anim)
    dur = (nf - 1) / fps
    K = max(2, int(round(dur * fps_out)) + 1)
    frames = []; mins = []
    ctx = R.heading_fix(*R.src_local(anim, 0))
    for k in range(K):
        f = k * (nf - 1) / (K - 1)
        p, q = R.src_local(anim, f)
        lp, lq, _w = R.retarget_frame(p, q, ctx)
        mins.append(R.foot_min(_w))
        frames.append((lp, [R.quat_euler(x) for x in lq]))
    # Altura: el primer cuadro se apoya en el suelo del MU (no arranca hundido) y,
    # si en algun momento los pies lo atraviesan, se sube el cuerpo lo justo.
    zref = R.mu_ground_ref()
    dz0 = zref - mins[0]
    for (lp, le), m in zip(frames, mins):
        dz = max(dz0, zref - m)
        x, y, z = lp[0]; lp[0] = (x, y, z + dz)
    # El plugin original reproduce los emotes a sm_emotes_speed = 0.84 (m_flPlaybackRate),
    # o sea 16 % mas lento que lo autorado; se replica para que duren y suenen igual.
    speed = (K - 1) * fps / ((nf - 1) * 25.0) * PLUGIN_PLAYBACK_RATE
    return frames, bool(flags & 1), speed

def main():
    sounds = {}
    if '--sp' in sys.argv:
        sounds = load_sounds(sys.argv[sys.argv.index('--sp') + 1])
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    if '--sp' in sys.argv: args.remove(sys.argv[sys.argv.index('--sp') + 1])
    if '--fps' in sys.argv: args.remove(sys.argv[sys.argv.index('--fps') + 1])
    fps_out = 20
    if '--fps' in sys.argv: fps_out = int(sys.argv[sys.argv.index('--fps') + 1])
    out, sel = args[0], args[1].split(',')
    seqs = sequences(); byname = {n.lower(): (n, a) for n, a in seqs}
    picks = []
    for s in sel:
        # variante opcional: nombre:clav=0.8;wrist=0.3  (amortiguacion de clavicula/muneca)
        opts = {}
        if ':' in s:
            s, o = s.split(':', 1)
            opts = dict(kv.split('=') for kv in o.split(';'))
        if s.isdigit(): n, a = seqs[int(s)]
        else: n, a = byname[s.lower()]
        tag = ''.join('_%s%d' % (k[0], round(float(v) * 100)) for k, v in opts.items())
        picks.append((n + tag, a, opts))
    acts = []; txt = []
    for n, a, opts in picks:
        R.CLAVICLE_DAMP = float(opts.get('clav', DEFAULTS[0]))
        R.WRIST_DAMP = float(opts.get('wrist', DEFAULTS[1]))
        R.TWIST = float(opts.get('twist', DEFAULTS[2]))
        frames, loop, speed = convert(a, fps_out)
        base = n.split('_c')[0] if False else n
        snd = SOUND_OVERRIDE.get(base) or sounds.get(base, '')
        acts.append(frames); txt.append('%s %d %.3f %s' % (n, 1 if loop else 0, speed, snd.replace(',', '|') or '-'))
        print('%-28s %3d frames  loop=%d  speed=%.3f' % (n, len(frames), loop, speed))
    B = R.B_mu
    o = bytearray(b'BMD\x0A') + b'CustomAnim'.ljust(32, b'\0')
    o += struct.pack('<hhh', 0, len(B), len(acts))
    for fr in acts: o += struct.pack('<hB', len(fr), 0)
    for i, bn in enumerate(B):
        if bn is None: o += b'\x01'; continue
        o += b'\x00' + bn[0].split(b'\0')[0].ljust(32, b'\0') + struct.pack('<h', bn[1])
        for fr in acts:
            for lp, le in fr: o += struct.pack('<3f', *lp[i])
            for lp, le in fr: o += struct.pack('<3f', *le[i])
    open(out, 'wb').write(bytes(o))
    open(os.path.splitext(out)[0] + '.txt', 'w').write('\n'.join(txt) + '\n')
    print('%s: %d acciones, %d bytes' % (out, len(acts), len(o)))

if __name__ == '__main__':
    main()
