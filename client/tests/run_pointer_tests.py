"""Corre en paralelo una lista de tests de punteros.

Cada línea del archivo es un test con sus argumentos, relativo a client/:

    tests/SkillPointer.py --architecture x64

Las líneas vacías y las que empiezan con # se ignoran. Cada test arma y compila
su propio proyecto en una carpeta temporal, así que se pueden correr a la vez.
La salida de cada test se muestra entera sólo si falla.
"""
import argparse
import os
import shlex
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

CLIENT = Path(__file__).resolve().parent.parent


def read_list(path):
    tests = []
    for line in Path(path).read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if line and not line.startswith("#"):
            tests.append(shlex.split(line))
    return tests


def run(test):
    start = time.monotonic()
    result = subprocess.run([sys.executable, "-B", *test], cwd=CLIENT,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            text=True, encoding="utf-8", errors="replace")
    return test, result.returncode, result.stdout, time.monotonic() - start


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("lists", nargs="+", help="archivos con un test por línea")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 2)
    args = parser.parse_args()

    tests = [test for path in args.lists for test in read_list(path)]
    # La consola de Windows usa cp1252 y los tests imprimen UTF-8.
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    os.environ["PYTHONIOENCODING"] = "utf-8"
    # MSBuild deja nodos vivos para reusarlos entre builds; con varios builds
    # en paralelo no hace falta y así cada test termina limpio.
    os.environ["MSBUILDDISABLENODEREUSE"] = "1"

    failures = []
    start = time.monotonic()
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = [pool.submit(run, test) for test in tests]
        for done, future in enumerate(as_completed(futures), 1):
            test, code, output, seconds = future.result()
            name = " ".join(test)
            if code:
                failures.append(name)
                print(f"::group::FALLA {name}", flush=True)
                print(output, flush=True)
                print("::endgroup::", flush=True)
            else:
                last = output.strip().splitlines()[-1] if output.strip() else ""
                print(f"[{done}/{len(tests)}] {seconds:5.1f}s {name}: {last}", flush=True)

    print(f"{len(tests)} tests en {time.monotonic() - start:.0f}s con {args.jobs} en paralelo")
    if failures:
        print(f"{len(failures)} fallaron:")
        for name in failures:
            print(f"  {name}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
