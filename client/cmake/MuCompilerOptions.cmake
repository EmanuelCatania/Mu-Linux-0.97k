# Sólo existen Debug y Release, como en el .sln.
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)

# Se vacían los flags por defecto de CMake (/RTC1, /Ob0, /Ob2, /DNDEBUG, /W1...,
# y -DWIN32/-D_DEBUG de rc.exe) para que sólo queden los que pone cada target,
# igual que en el .vcxproj.
foreach(lang C CXX RC)
    set(CMAKE_${lang}_FLAGS "")
    set(CMAKE_${lang}_FLAGS_DEBUG "")
    set(CMAKE_${lang}_FLAGS_RELEASE "")
endforeach()
foreach(kind EXE STATIC SHARED MODULE)
    set(CMAKE_${kind}_LINKER_FLAGS "")
    set(CMAKE_${kind}_LINKER_FLAGS_DEBUG "")
    set(CMAKE_${kind}_LINKER_FLAGS_RELEASE "")
endforeach()

# Opciones de compilación comunes a mu97k y libjpeg, equivalentes a los defaults
# de v143 que el .vcxproj no pisa (msbuild las pasa explícitas a cl):
#   /EHsc  ExceptionHandling=Sync          /FC  UseFullPaths (afecta a __FILE__)
#   /Gd /Gm- /GS /Zc:forScope /Zc:inline /Zc:wchar_t   defaults de v143
#   /JMC   SupportJustMyCode (Debug)       /Od  Optimization=Disabled (Debug)
#   /O2 /Oi /Gy /GL  MaxSpeed + IntrinsicFunctions + FunctionLevelLinking +
#                    WholeProgramOptimization (Release)
set(MU_COMMON_COMPILE_OPTIONS
    /EHsc
    /FC
    /Gd
    /Gm-
    /GS
    /Zc:forScope
    /Zc:inline
    /Zc:wchar_t
    $<$<CONFIG:Debug>:/JMC>
    $<$<CONFIG:Debug>:/Od>
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Release>:/Oi>
    $<$<CONFIG:Release>:/Gy>
    $<$<CONFIG:Release>:/GL>
)

# UseDebugLibraries=true en Debug -> /MDd; Release -> /MD.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
# Debug -> /ZI (EditAndContinue, default de v143 en Debug); Release -> /Zi.
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
    "$<IF:$<CONFIG:Debug>,EditAndContinue,ProgramDatabase>")

