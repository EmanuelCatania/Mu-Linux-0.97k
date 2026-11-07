#!/usr/bin/env python3
"""Expresiones reales de 30 consumidores HFONT y nombre completo con handles altos.

No ejecuta los 30 flujos de UI ni GDI/FreeType reales. Comprueba conservación
del handle en cada expresión y usa el cuerpo real de CFont::GetFont.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from ItemNamePointer import harness as name_harness
from SceneObjectPointer import build_and_run, read_source

SITES = {
    "src/Input/Input_Timing.cpp": [0],
    "src/Item/Item_Display.cpp": [0],
    "src/Scene/Scene_CharSelect.cpp": [0,1,2,3,4],
    "src/Scene/Scene_Intro.cpp": [0],
    "src/Scene/Scene_Login.cpp": [0],
    "src/UI/CharMenu_Build.cpp": [0,1,2,3],
    "src/UI/Chat.cpp": [1,2,3],
    "src/UI/Quest_Legacy.cpp": [2,3,4],
    "src/UI/UI_GuildLegacy.cpp": [0],
    "src/UI/UI_LegacyExterns.cpp": [0],
    "src/UI/UI_StatsPanel.cpp": [0,1,2,3,4,5,6,7,8],
}
OLD = r"\((?:HGDIOBJ|HFONT)\)\((?:uintptr_t|DWORD)\)\(\(DWORD\)\(uintptr_t\)gFont\.GetFont\(FONT_\w+\)\)"
EXPRESSION = re.compile(OLD+r"|(?:\((?:HGDIOBJ|HFONT)\))?gFont\.GetFont\(FONT_\w+\)")

MAIN = r'''
int main() {
    std::uint64_t digest=14695981039346656037ull;
    for(unsigned trial=0;trial<4096;++trial) {
        const std::uintptr_t base=trial==0?0:std::uintptr_t(0x1230+trial*16);
        for(int font=0;font<3;++font) gFont.m_Fonts[font]=reinterpret_cast<HFONT>(base?base+font:0);
        for(int site=0;site<30;++site) {
            const auto result=ReadHandle(site),expected=reinterpret_cast<std::uintptr_t>(gFont.m_Fonts[types[site]]);
            Require(result==expected); digest^=result; digest*=1099511628211ull;
        }
        if(sizeof(void*)>4 && base) for(std::uint64_t high:{0x100000000ull,0x17fff0000ull,0x200000000ull}) {
            for(int font=0;font<3;++font) gFont.m_Fonts[font]=reinterpret_cast<HFONT>(high+base+font);
            for(int site=0;site<30;++site) Require(ReadHandle(site)==high+base+types[site]);
        }
    }
    Require(gFont.GetFont(static_cast<eFontTypes>(-1))==nullptr && gFont.GetFont(static_cast<eFontTypes>(3))==nullptr);
    std::printf("HFONT: 30 sitios/11 archivos, 4096 handles nulos/bajos y altos %016llx\n",static_cast<unsigned long long>(digest));
}
'''


def harness(revision=None):
    prefix=PREAMBLE.split("char DAT_00559bb4",1)[0]+"\n#ifndef _WIN32\nusing HFONT=void*; using HGDIOBJ=void*;\n#endif\nusing std::uintptr_t;\n"
    header=read_source("src/Core/Font.h",revision)
    prefix+=re.search(r"enum eFontTypes\s*\{.*?\};",header,re.S).group()
    getter=re.search(r"HFONT GetFont\(eFontTypes type\) const\s*\{.*?\n    \}",header,re.S).group()
    prefix+="\nstruct TestFont { HFONT m_Fonts[MAX_FONT_TYPES]; "+getter+" }; TestFont gFont;\n"
    expressions=[]
    for path,indices in SITES.items():
        source=re.sub(r"//[^\n]*|/\*.*?\*/","",read_source(path,revision),flags=re.S)
        calls=list(EXPRESSION.finditer(source))
        expressions.extend(calls[i].group() for i in indices)
    if len(expressions)!=30: raise ValueError("Se esperaban 30 expresiones")
    types=[re.search(r"FONT_(NORMAL|BOLD|BIG)",expression).group() for expression in expressions]
    prefix+="\nint types[]={"+",".join(types)+"};\nstd::uintptr_t ReadHandle(int site) { switch(site) {\n"
    prefix+="\n".join(f"case {i}: return reinterpret_cast<std::uintptr_t>({expression});" for i,expression in enumerate(expressions))
    return prefix+"\n} Require(false); return 0; }\n"+MAIN


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64")
    parser.add_argument("--generator"); parser.add_argument("--reference-ref")
    args=parser.parse_args()
    if args.reference_ref and (os.name!="nt" or args.architecture!="Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-font-handle-") as temp:
        directory=Path(temp)
        current=build_and_run(directory/"current",harness(),args); print(current,end="")
        names=build_and_run(directory/"names",name_harness(high_fonts=True),args); print(names,end="")
        if args.reference_ref:
            reference=build_and_run(directory/"reference",harness(args.reference_ref),args)
            old_names=build_and_run(directory/"old-names",name_harness(args.reference_ref,high_fonts=True),args)
            if current!=reference or names!=old_names: raise RuntimeError("Diferencia con referencia x86")
            print(f"Equivalencia con {args.reference_ref}: OK")


if __name__=="__main__":
    main()
