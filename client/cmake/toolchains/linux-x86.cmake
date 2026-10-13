# Cliente de 32 bits. El backend Linux todavía no está implementado.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR i686)
set(CMAKE_C_FLAGS_INIT "-m32 -fsigned-char -mfpmath=sse -msse2")
set(CMAKE_CXX_FLAGS_INIT "-m32 -fsigned-char -mfpmath=sse -msse2")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-m32")
