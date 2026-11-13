#!/usr/bin/env python3
"""Verifica que los fuentes que comparten cliente y server sean idénticos.

El canal cifrado (SecureChannel) y Monocypher están copiados en los dos árboles:
si una copia cambia y la otra no, las dos puntas dejan de entenderse.

Sale con código 1 si alguna pareja difiere. Lo corre el CI.

Uso: python .github/scripts/check_shared_sources.py
"""
import os
import sys

CLIENT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SERVER = os.path.join(os.path.dirname(CLIENT), "server", "Source", "MuServer", "Common")

PAIRS = [
    ("src/net/SecureChannel.h", "SecureChannel.h"),
    ("src/net/SecureChannel.cpp", "SecureChannel.cpp"),
    ("src/net/crypto/SecureRandom.h", "SecureRandom.h"),
    ("src/net/crypto/monocypher.h", "monocypher/monocypher.h"),
    ("src/net/crypto/monocypher.c", "monocypher/monocypher.c"),
]


def content(path):
    with open(path, "rb") as f:
        return f.read().replace(b"\r\n", b"\n")


def main():
    if not os.path.isdir(SERVER):
        print("check_shared_sources: no está el árbol del server, se omite")
        return 0
    failed = False
    for client, server in PAIRS:
        a = os.path.join(CLIENT, client)
        b = os.path.join(SERVER, server)
        if content(a) != content(b):
            print("DIFIEREN: client/%s  y  server/Source/MuServer/Common/%s" % (client, server))
            failed = True
        else:
            print("[compartido] %s: OK" % client)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
