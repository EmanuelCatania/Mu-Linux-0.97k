#!/usr/bin/env python3
"""Contratos de tela contra los offsets x86 de IDA y su expansión nativa."""
import argparse
import tempfile
from pathlib import Path
from SceneObjectPointer import build_and_run, read_source


def harness():
    layouts = {
        "ClothNode": (["VTable", "Acceleration", "Velocity", "Position", "Flags", "Padding", "CorrectionCount", "Correction"], [0,4,16,28,40,41,44,48], [0], 60),
        "ClothAnchor": (["VTable", "LocalPosition", "Bone", "WorldPosition", "Radius"], [0,4,16,20,32], [0], 36),
        "ClothLink": (["Value", "Previous", "Next"], [0,4,8], [0,4,8], 12),
        "ClothSpring": (["A", "B", "Minimum", "Rest", "Flags", "Padding"], [0,2,4,8,12,13], [], 16),
        "ClothWidget": (["VTable", "Owner", "Bone", "FrontTexture", "BackTexture", "Flags", "OriginU", "OriginV", "Width", "Height", "Columns", "Rows", "NodeCount", "Nodes", "SpringCount", "Springs", "StepU", "StepV", "AnchorCount", "Head", "Tail"], list(range(0,84,4)), [0,4,52,60,76,80], 84),
        "ClothModelMesh": (["Unknown0","VertexCount","NormalCount","TexCoordCount","TriangleCount","Unknown12","Vertices","Normals","TexCoords","Triangles","Unknown32","TextureFlags"], [0,4,6,8,10,12,16,20,24,28,32,36], [16,20,24,28,36], 40),
    }
    assertions = []
    for name, (fields, offsets, pointers, size) in layouts.items():
        assertions.append(f"static_assert(sizeof({name})=={size}+{len(pointers)}*(sizeof(void*)-4));")
        assertions.append(f"static_assert(alignof({name})==1 && std::is_standard_layout_v<{name}> && std::is_trivially_copyable_v<{name}>);")
        for field, offset in zip(fields, offsets):
            before = sum(p < offset for p in pointers)
            assertions.append(f"static_assert(offsetof({name},{field})=={offset}+{before}*(sizeof(void*)-4));")
    header = read_source("src/Physics/ClothTypes.h").replace("#pragma once", "")
    return header + "\n" + "\n".join(assertions) + r'''
#include <cstdio>
static_assert(offsetof(ClothMeshWidget,MeshIndex)==sizeof(ClothWidget));
static_assert(offsetof(ClothMeshWidget,Unknown)==sizeof(ClothWidget)+4);
int main(){std::printf("ClothLayoutContract ptr=%zu: widget=%zu mesh=%zu node=%zu anchor=%zu link=%zu spring=%zu OK\n",
sizeof(void*),sizeof(ClothWidget),sizeof(ClothMeshWidget),sizeof(ClothNode),sizeof(ClothAnchor),sizeof(ClothLink),sizeof(ClothSpring));}
'''


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator")
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-cloth-contract-") as temp:
        print(build_and_run(Path(temp)/"current",harness(),args),end="")


if __name__=="__main__":main()
