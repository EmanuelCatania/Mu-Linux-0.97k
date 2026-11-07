# Cargador OpenGL

`glad.h`: glad **2.0.8**, OpenGL **3.3 core**, sin extensiones, generado con la especificación incluida en esa versión (`--reproducible`). El archivo generado y sus avisos de licencia se conservan sin modificaciones. No se necesita Python ni acceso de red para compilar el cliente.

Origen: https://github.com/Dav1dde/glad/tree/v2.0.8

Regeneración (entorno de herramientas separado):

```
python -m pip install glad2==2.0.8
python -m glad --api gl:core=3.3 --extensions= --reproducible --out-path generated c --header-only
```

Copiar `generated/include/glad/gl.h` como `glad.h`. `LICENSE.glad` conserva las licencias del generador y las especificaciones; el header incorpora también los avisos de los tipos Khronos.

El cargador se implementa una sola vez, en `Platform/GLFunctions.cpp`, sin PCH ni `GL/gl.h`. El cliente no usa el pipeline fijo: con el header core, cualquier llamada de OpenGL 1.1 deja de compilar. De `opengl32` sólo se usa WGL (creación de contexto en Win32).
