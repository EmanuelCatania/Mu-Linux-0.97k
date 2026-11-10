#!/usr/bin/env python3
"""Genera PacketNames.inc, la tabla de nombres de paquetes que usa mu-trace.

- Recibidos: el switch de ProtocolCore (GameServer/Protocol.cpp). El nombre es
  la función a la que despacha cada case, incluidos los sub-switches por
  lpMsg[3] (F1, F3, ...).
- Enviados: cada `.set(...)` / `.setE(...)` de una cabecera en los .cpp del
  GameServer. El nombre es la función que arma el paquete; si varias arman el
  mismo opcode se guarda la primera y se marca con "+".

Uso: python3 gen_packet_names.py  (regenera el archivo al lado del script)
"""

import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
GS = os.path.normpath(os.path.join(HERE, "..", "GameServer"))


def read(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read()


def load_codes():
    codes = {}
    for m in re.finditer(r"#define\s+(PROTOCOL_CODE\d)\s+(0x[0-9A-Fa-f]+)", read(os.path.join(GS, "Protocol.h"))):
        codes[m.group(1)] = int(m.group(2), 16)
    return codes


def to_int(tok, codes):
    tok = tok.strip()
    if tok in codes:
        return codes[tok]
    try:
        return int(tok, 0)
    except ValueError:
        return None


def short(name):
    name = name.split("::")[-1]
    for pre in ("CG", "GC"):
        if name.startswith(pre) and len(name) > 2 and name[2].isupper():
            name = name[2:]
    for suf in ("Recv", "Send"):
        if name.endswith(suf) and len(name) > len(suf):
            name = name[: -len(suf)]
    return name


def recv_names(codes):
    src = read(os.path.join(GS, "Protocol.cpp"))
    start = src.index("void ProtocolCore(")
    body = src[start:]
    out = {}
    depth = 0
    stack = []  # (depth, head) de los switch anidados por lpMsg[3]
    head = None
    pending = None  # (head, sub) a la espera del nombre
    for line in body.splitlines():
        m = re.match(r"\s*case\s+([A-Za-z0-9_]+)\s*:", line)
        if m:
            val = to_int(m.group(1), codes)
            if stack and depth > stack[-1][0]:
                pending = (stack[-1][1], val)
            else:
                head = val
                pending = (val, -1)
        if re.search(r"switch\s*\(.*lpMsg\[3\]", line) and head is not None:
            stack.append((depth, head))
            out.pop((head, -1), None)
            pending = None
        if pending:
            c = re.search(r"([A-Za-z_][A-Za-z0-9_:.]*)\s*\(", line)
            if c and not line.strip().startswith(("case", "switch", "if", "//")):
                name = c.group(1).split(".")[-1]
                out.setdefault(pending, short(name))
                pending = None
        depth += line.count("{") - line.count("}")
        while stack and depth < stack[-1][0]:
            stack.pop()
        if depth <= 0 and "{" not in line and out:
            break
    return out


FUNC_RE = re.compile(r"^[A-Za-z_][\w:<>\*\s&]*?\b([A-Za-z_][\w:]*)\s*\([^;{]*\)\s*$")
SET_RE = re.compile(r"\b(\w+)(?:\.header)?\.set(E)?\(([^;]*)\)\s*;")
# Envíos al cliente: la función global DataSend y sus variantes. Las que van al
# DataServer o al JoinServer se llaman como método (gDataServerConnection.DataSend)
# y quedan afuera por el "." que las precede.
CLIENT_SEND = r"(?<![\w.])(?:DataSend|DataSendAll|MsgSendV2|DataSendToAll)\s*\([^;]*&\s*%s\b"

CLIENT_ANY = CLIENT_SEND.split(r"\(")[0] + r"\s*\("
SERVER_LINK = r"g(?:DataServer|JoinServer)Connection\."

def functions(text):
    """Devuelve (nombre, cuerpo) de cada función definida a nivel de archivo."""
    lines = text.splitlines()
    i = 0
    while i < len(lines):
        m = FUNC_RE.match(lines[i])
        if m and not lines[i].startswith(("\t", " ", "#", "//", "return", "else")) \
                and i + 1 < len(lines) and lines[i + 1].startswith("{"):
            j = i + 2
            while j < len(lines) and not lines[j].startswith("}"):
                j += 1
            yield m.group(1), "\n".join(lines[i + 2:j])
            i = j
        i += 1


def send_names(codes):
    out = {}
    for fn in sorted(os.listdir(GS)):
        if not fn.endswith(".cpp"):
            continue
        for func, body in functions(read(os.path.join(GS, fn))):
            for s in SET_RE.finditer(body):
                var = s.group(1)
                # El paquete se manda con &var, o se copia a un buffer y la
                # función sólo habla con el cliente (sin conexiones a otros servers).
                if not re.search(CLIENT_SEND % re.escape(var), body) and not (
                        re.search(CLIENT_ANY, body) and not re.search(SERVER_LINK, body)):
                    continue
                args = [a.strip() for a in s.group(3).split(",")]
                head = to_int(args[0], codes)
                if head is None:
                    continue
                sub = -1
                if len(args) == 3:
                    sub = to_int(args[1], codes)
                    if sub is None:
                        sub = -2  # sub calculado en tiempo de ejecución
                key = (head, sub)
                name = short(func)
                if key in out and out[key] != name and not out[key].endswith("+"):
                    out[key] = out[key] + "+"
                else:
                    out.setdefault(key, name)
    return out


def emit(f, table, label):
    f.write("static const PacketName k%sNames[] = {\n" % label)
    for (head, sub), name in sorted(table.items(), key=lambda kv: (kv[0][0], kv[0][1])):
        f.write('\t{ 0x%02X, %d, "%s" },\n' % (head, sub, name))
    f.write("};\n\n")


def main():
    codes = load_codes()
    recv = recv_names(codes)
    send = send_names(codes)
    if not recv or not send:
        sys.exit("no se pudo armar la tabla (recv=%d, send=%d)" % (len(recv), len(send)))
    path = os.path.join(HERE, "PacketNames.inc")
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("// Generado por gen_packet_names.py; no editar a mano.\n")
        f.write("// sub = -1: el opcode no tiene subcódigo; -2: el subcódigo se calcula al enviar.\n\n")
        emit(f, recv, "Recv")
        emit(f, send, "Send")
    print("PacketNames.inc: %d recibidos, %d enviados" % (len(recv), len(send)))


if __name__ == "__main__":
    main()
