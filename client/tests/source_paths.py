"""Carpetas de src/ que cambiaron de nombre (Fase 5, E3).

Los tests que leen el código de una revisión vieja (`git show REV:ruta`) piden
la ruta nueva; si en esa revisión no existe, se prueba la de antes.
"""
import subprocess

# nueva -> vieja, como prefijos de ruta relativos a client/.
MOVED = {
    "src/game/math/": "src/Math/",
    "src/game/trade/": "src/Trade/",
    "src/game/physics/": "src/Physics/",
    "src/game/path/": "src/Path/",
    "src/game/party/": "src/Party/",
    "src/audio/": "src/Sound/",
}


def candidates(path):
    """La ruta pedida y, si estaba en una carpeta movida, la de antes."""
    out = [path]
    for new, old in MOVED.items():
        if path.startswith(new):
            out.append(old + path[len(new):])
    return out


def show(root, revision, path):
    """`git show REV:ruta` probando la ruta vieja si la nueva no existe."""
    error = None
    for candidate in candidates(path):
        try:
            return subprocess.check_output(["git", "show", f"{revision}:{candidate}"], cwd=root,
                                           stderr=subprocess.DEVNULL).decode("utf-8")
        except subprocess.CalledProcessError as e:
            error = e
    raise error
