# Arquitetura

O repositório separa código-fonte, arquivos necessários em runtime, serviços auxiliares
e recursos de deployment.

```text
src/client ──build Windows──> C:\Dev\runtime\mu-097k\client
     │                              ▲
     └──InfoEncoder + MainInfo.ini──┘

src/server ──build Docker──> mu-server ──> MySQL
runtime/server───────────────┘       └──> services/web
```

## Cliente

`runtime/client` e `runtime/encoder` são templates rastreados. A automação copia esses
templates para um runtime externo, compila `Main.dll` e `InfoEncoder.exe`, executa o
encoder e instala `ClientInfo.bmd`. Binários rastreados nunca são sobrescritos.

## Servidor

O Dockerfile compila `src/server` com CMake e copia somente os binários resultantes e
os dados de `runtime/server` para a imagem final. MySQL, servidor e painel web
compartilham apenas a rede privada padrão do Compose.

## Serviços opcionais

O painel web faz parte do stack base. O editor é habilitado com o overlay
`compose.editor.yaml` e volumes explícitos; ele não é necessário para executar o
servidor local básico.
