crypt_blowfish 1.3 (Solar Designer, Openwall), dominio público.

Origen: https://www.openwall.com/crypt/crypt_blowfish-1.3.tar.gz
(sha256 83fa01fca6996fe8d882b7f8e9ba0305a5664936100b01481ea3c6a8ce8d72fd).

Sólo se copian `crypt_blowfish.c` y `crypt_blowfish.h`. Único cambio: la
versión en ensamblador para i386 (`x86.S`, que no se trae) queda detrás de
`BF_USE_X86_ASM`; sin esa definición se compila la versión en C en todas las
plataformas.

Lo usa `Common/PasswordHash` para guardar las contraseñas de cuenta con bcrypt
(`$2b$`), compatible con `bcryptjs` del panel web.
