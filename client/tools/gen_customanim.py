#!/usr/bin/env python3
"""Genera Data/Player/CustomAnim.bmd: acciones extra para el modelo del jugador.

El archivo es un BMD v10 (sin cifrar) con 0 mallas, los mismos huesos que
player.bmd y N acciones. El cliente las agrega al final del modelo del jugador
(ver Model/Model_CustomAnim.cpp), por lo que no pisan player.bmd y sobreviven
a un cambio de version del mismo.

Uso:
  gen_customanim.py <player_destino.bmd> <player_fuente.bmd> <idx,idx-idx,...> <salida.bmd>
Los indices se toman del player_fuente. Los huesos se validan por nombre contra
el player_destino.
"""
import struct, sys

K = bytes.fromhex('d17352f6d29acb273eaf593137b3e7a2')

def payload(path):
    d = open(path, 'rb').read()
    assert d[:3] == b'BMD', path
    if d[3] == 0x0C:
        n = struct.unpack_from('<i', d, 4)[0]; w = 0x5E; o = bytearray()
        for i, b in enumerate(d[8:8 + n]):
            o.append(((b ^ K[i & 15]) - w) & 255); w = (b + 0x3D) & 255
        return bytes(o)
    assert d[3] == 0x0A, 'version no soportada'
    return d[4:]

def load(path):
    b = payload(path)
    nm, nb, na = struct.unpack_from('<hhh', b, 32); p = 38
    for _ in range(nm):
        v, n, t, tr, _x = struct.unpack_from('<hhhhh', b, p); p += 10
        p += v * 16 + n * 20 + t * 8 + tr * 64 + 32
    acts = []
    for _ in range(na):
        fc, lk = struct.unpack_from('<hB', b, p); p += 3
        pos = b''
        if lk: pos = b[p:p + fc * 12]; p += fc * 12
        acts.append([fc, lk, pos])
    bones = []
    for _ in range(nb):
        dummy = b[p]; p += 1
        if dummy: bones.append(None); continue
        name = b[p:p + 32]; par = struct.unpack_from('<h', b, p + 32)[0]; p += 34
        keys = []
        for fc, lk, _pos in acts:
            kp = b[p:p + fc * 12]; p += fc * 12
            kr = b[p:p + fc * 12]; p += fc * 12
            keys.append((kp, kr))
        bones.append((name, par, keys))
    return acts, bones

def parse_idx(s):
    out = []
    for part in s.split(','):
        if '-' in part:
            a, b = part.split('-'); out += range(int(a), int(b) + 1)
        else: out.append(int(part))
    return out

def main():
    dst, src, idx, out = sys.argv[1:5]
    idx = parse_idx(idx)
    _, bd = load(dst)
    sa, bs = load(src)
    assert len(bd) == len(bs), 'cantidad de huesos distinta'
    for i, (x, y) in enumerate(zip(bd, bs)):
        assert (x is None) == (y is None), 'hueso dummy distinto %d' % i
        if x: assert x[0].split(b'\x00')[0] == y[0].split(b'\x00')[0] and x[1] == y[1], 'hueso %d distinto' % i
    o = bytearray(b'BMD\x0A')
    o += b'CustomAnim'.ljust(32, b'\0')
    o += struct.pack('<hhh', 0, len(bs), len(idx))
    for a in idx:
        fc, lk, pos = sa[a]
        o += struct.pack('<hB', fc, lk) + pos
    for bn in bs:
        if bn is None: o += b'\x01'; continue
        o += b'\x00' + bn[0] + struct.pack('<h', bn[1])
        for a in idx: o += bn[2][a][0] + bn[2][a][1]
    open(out, 'wb').write(bytes(o))
    print('CustomAnim.bmd: %d acciones, %d bytes' % (len(idx), len(o)))
    for n, a in enumerate(idx): print('  custom %d <- fuente %d (%d frames, lock=%d)' % (n, a, sa[a][0], sa[a][1]))

if __name__ == '__main__':
    main()
