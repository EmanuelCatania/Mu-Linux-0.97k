"""Fija sólo los slots de tela en bancos que certifican el layout legacy x86.

Esos bancos no prueban el ABI nativo de OBJECT/PART_t. ClothRuntime y
ObjectLayoutContract usan las declaraciones reales sin esta adaptación.
"""
import re


def opaque_cloth_fields(text):
    return re.sub(r'\bvoid\s*\*\s+Cloth\b','std::uint32_t Cloth',text)
