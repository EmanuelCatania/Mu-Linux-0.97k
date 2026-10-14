# Cliente de 32 bits. SDL3 se valida primero como biblioteca; el ejecutable se habilita por etapas.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR i686)
set(CMAKE_C_FLAGS_INIT "-m32 -fsigned-char -mfpmath=sse -msse2")
set(CMAKE_CXX_FLAGS_INIT "-m32 -fsigned-char -mfpmath=sse -msse2")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-m32")
