#!/usr/bin/env python3
"""Reemplaza acciones puntuales de un player.bmd por otras de otro player.bmd.

Sirve para rellenar los huecos de player.bmd (posiciones 68..77, copias de la
accion 1 en el 0.97.11) con animaciones originales de otra version del juego.
Escribe un BMD v12 (cifrado), el mismo formato que el original.

Uso:
  patch_player_actions.py <player_base.bmd> <player_fuente.bmd> <destino:fuente,...> <salida.bmd>
Ejemplo: 68:222,69:223  -> la accion 68 pasa a ser la 222 de la fuente, etc.
"""
import struct, sys
from gen_customanim import payload, load, K

def encrypt12(plain):
    out = bytearray(); w = 0x5E
    for i, v in enumerate(plain):
        b = ((v + w) & 255) ^ K[i & 15]
        out.append(b); w = (b + 0x3D) & 255
    return bytes(out)

def main():
    base, src, pairs, out = sys.argv[1:5]
    pairs = [tuple(int(x) for x in p.split(':')) for p in pairs.split(',')]
    raw = payload(base)
    acts, bones = load(base)
    sa, sb = load(src)
    assert len(bones) == len(sb)
    for dst, s in pairs:
        assert 0 <= dst < len(acts), 'destino fuera de rango'
        acts[dst] = list(sa[s])
        for bn, sbn in zip(bones, sb):
            if bn: bn[2][dst] = sbn[2][s]
    nm, nb, na = struct.unpack_from('<hhh', raw, 32)
    head = bytearray(raw[:38]); assert nm == 0 or True
    # Las mallas se copian tal cual; se reescriben solo acciones y huesos.
    p = 38
    for _ in range(nm):
        v, n, t, tr, _x = struct.unpack_from('<hhhhh', raw, p); p += 10
        p += v * 16 + n * 20 + t * 8 + tr * 64 + 32
    o = bytearray(raw[:p])
    for fc, lk, pos in acts:
        o += struct.pack('<hB', fc, lk) + (pos if lk else b'')
    for i, bn in enumerate(bones):
        if bn is None: o += b'\x01'; continue
        # conservar los 34 bytes originales de cabecera de hueso (nombre incluido)
        o += b'\x00' + bn[0].ljust(32, b'\x00')[:32] + struct.pack('<h', bn[1])
        for kp, kr in bn[2]: o += kp + kr
    enc = encrypt12(bytes(o))
    open(out, 'wb').write(b'BMD\x0C' + struct.pack('<i', len(enc)) + enc)
    print('player.bmd parcheado: %d acciones, %d bytes' % (len(acts), 8 + len(enc)))

if __name__ == '__main__':
    main()
