#!/usr/bin/env python3
"""Contrato de celdas nativas: bytes, canarios y punteros altos en las 256 celdas."""
import argparse
import tempfile
from pathlib import Path
from SceneObjectPointer import build_and_run, read_source


def harness():
    header = read_source("src/Scene/SceneObjectCell.h").replace("#pragma once", "")
    return header + r'''
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
void Require(bool value) { if (!value) std::abort(); }
struct Bank { std::uint8_t before[32]; SceneObjectCell cells[256]; std::uint8_t after[32]; };
static Bank bank;
static std::uint8_t owners[512];
int main() {
    if (sizeof(void*) == 8)
        Require(reinterpret_cast<std::uintptr_t>(owners) > 0xffffffffull);
    unsigned cases = 0;
    for (unsigned fill = 0; fill < 256; ++fill) {
        std::memset(&bank, fill, sizeof(bank));
        auto expected = std::array<std::uint8_t, sizeof(Bank)>{};
        std::memcpy(expected.data(), &bank, sizeof(bank));
        for (unsigned cell = 0; cell < 256; ++cell) {
            auto* head = static_cast<void*>(&owners[cell * 2]);
            auto* tail = static_cast<void*>(&owners[cell * 2 + 1]);
            const std::uint8_t visible = static_cast<std::uint8_t>(cell);
            auto* bytes = expected.data() + offsetof(Bank, cells) + cell * sizeof(SceneObjectCell);
            // Oráculo de offsets independiente de offsetof de los campos comprobados.
            std::memcpy(bytes + 4, &head, sizeof(head));
            std::memcpy(bytes + 4 + sizeof(void*), &tail, sizeof(tail));
            bytes[4 + 2 * sizeof(void*)] = visible;
            bank.cells[cell].Head = head;
            bank.cells[cell].Tail = tail;
            bank.cells[cell].Visible = visible;
            Require(std::memcmp(expected.data(), &bank, sizeof(bank)) == 0);
            ++cases;
        }
        for (unsigned cell = 0; cell < 256; ++cell) {
            Require(bank.cells[cell].Head == &owners[cell * 2]);
            Require(bank.cells[cell].Tail == &owners[cell * 2 + 1]);
            Require(bank.cells[cell].Visible == cell);
            Require(bank.cells[cell].Reserved == 0x01010101u * fill);
        }
    }
    std::printf("SceneObjectCell: %u casos, celda=%zu, grilla=%zu, puntero=%zu OK\n",
                cases, sizeof(SceneObjectCell), sizeof(bank.cells), sizeof(void*));
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-scene-cell-") as temp:
        print(build_and_run(Path(temp) / "current", harness(), args), end="")


if __name__ == "__main__":
    main()
