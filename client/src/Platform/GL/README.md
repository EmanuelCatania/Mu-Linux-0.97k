# Cargador OpenGL

`glad.h`: glad **2.0.8**, OpenGL **3.3 compatibility**, sin extensiones, generado con la especificación incluida en esa versión (`--reproducible`). El archivo generado y sus avisos de licencia se conservan sin modificaciones. No se necesita Python ni acceso de red para compilar el cliente.

Origen: https://github.com/Dav1dde/glad/tree/v2.0.8

Regeneración (entorno de herramientas separado):

```
python -m pip install glad2==2.0.8
python -m glad --api gl:compatibility=3.3 --extensions= --reproducible --out-path generated c --header-only
```

Copiar `generated/include/glad/gl.h` como `glad.h`. `LICENSE.glad` conserva las licencias del generador y las especificaciones; el header incorpora también los avisos de los tipos Khronos.

El cargador se implementa una sola vez, en `Platform/GLFunctions.cpp`, sin PCH ni `GL/gl.h`. Las llamadas heredadas siguen enlazando con OpenGL 1.1; el código moderno puede incluir este header antes de cualquier header GL y usar los símbolos cargados.
