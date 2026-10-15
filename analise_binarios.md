# Análise Binária - Arquivos DEX e SO

## 1. Arquivo: `7d0a794f-classes4.dex`

### Informações Gerais

- **Tipo**: Dalvik DEX file version 035 (Android bytecode)
- **Tamanho**: ~1.3 MB (estimado)
- **Arquitetura**: Java/Android

### Classes Identificadas

#### Pacote de Debugging (ADRT - Android Remote Debugging Tools)

- **Classe**: `adrt/ADRTSender`
  - Responsável por enviar dados de debugging via broadcast
  - Métodos principais:
    - `sendBreakpointHit()` - notifica breakpoints
    - `sendConnect()` - estabelece conexão com debugger
    - `sendFields()` - envia campos de objetos
    - `sendLogcatLines()` - envia linhas de logcat
    - `sendStop()` - encerra debugging
  - Usa `Intent` e `Context` para comunicação
  - Broadcasting para: `com.adrt.BREAKPOINT_HIT`, `com.adrt.CONNECT`, `com.adrt.FIELDS`, `com.adrt.LOGCAT_ENTRIES`, `com.adrt.STOP`

- **Classe**: `adrt/ADRTLogCatReader`
  - Lê logs em tempo real do sistema
  - Executa comando: `logcat -v threadtime`
  - Herda de `Runnable` para leitura assíncrona
  - Lê via `BufferedReader` do processo logcat

#### Aplicação Principal

- **Classe**: `com/mycompany/application/MainActivity`
  - Activity principal
  - Gerencia `GLSurfaceView` com listener para toque
  - Classe anônima `MainActivity$100000000` implementa `OnTouchListener`
  - Classe anônima `MainActivity$100000001` (Handler para delays)
  - Métodos: `onCreate()`, manipulação de eventos de toque
  - Acessa recursos do aplicativo via `R.*`

#### View OpenGL

- **Classe**: `com/mycompany/application/GLES3JNIView`
  - Extends `GLSurfaceView`
  - Implementa `GLSurfaceView.Renderer`
  - Métodos JNI:
    - `init()` - inicializa renderer
    - `step()` - frame rendering
    - `onSurfaceCreated()` - setup de superfície
    - `onSurfaceChanged()` - responde a mudanças de tamanho
    - `onDrawFrame()` - renderiza cada frame
    - `MotionEventClick()` - processa eventos de toque
    - `getWindowRect()` - obtém dimensões da janela
    - `imgui_Shutdown()` - shutdown ImGui
    - `resize()` - redimensiona viewport
  - Carrega biblioteca nativa: `loadLibrary("matveuxxx")`

#### Recursos (R.java)

- **Cores**: `colorAccent`, `colorPrimary`, `colorPrimaryDark`
- **Temas**: `AppTheme`, `DarkApp`
- **Layouts**: `activity_main`
- **Strings**: `app_name`

### Dependências Encontradas

- Android Framework (Intent, Context, View, Activity)
- OpenGL ES (GLSurfaceView, Renderer)
- I/O e Concorrência (BufferedReader, Runtime, Thread, Handler)
- Reflexão (Class, Method, invoke)
- Coleções (ArrayList, Iterator)

---

## 2. Arquivo: `81b9e9e1-libmatveuxxx.so`

### Informações Gerais

- **Tipo**: ELF 64-bit LSB shared object
- **Arquitetura**: ARM aarch64
- **Estado**: Stripped (símbolos de debug removidos)
- **BuildID**: `0aa41dd7292b9925983065c983ac25de2c21f020`
- **Tamanho Code**: ~1.35 MB (estimado)
- **Tamanho Data**: ~1.05 MB (estimado)

### Dependências Dinâmicas

```
liblog.so       - Android logging API
libz.so         - Compressão/descompressão zlib
libandroid.so   - Android native APIs
libEGL.so       - EGL (OpenGL ES interface layer)
libGLESv2.so    - OpenGL ES 2.0
libGLESv3.so    - OpenGL ES 3.0
libc.so         - C runtime library (POSIX)
libm.so         - Matemática (libm)
libdl.so        - Dynamic linking
```

### Funções Exportadas (JNI Interface)

#### Inicialização

- `JNI_OnLoad()` @0x8366c
  - Chamada na carregamento da biblioteca
  - Registra métodos JNI nativos

- `JNI_OnUnload()` @0x836f0
  - Chamada na descarregamento

#### Renderização OpenGL (ImGui)

- `Java_com_mycompany_application_GLES3JNIView_init()` @0x79e54
  - Inicializa ImGui e OpenGL ES

- `Java_com_mycompany_application_GLES3JNIView_step()` @0x79fec
  - Renderiza frame (animation loop)

- `Java_com_mycompany_application_GLES3JNIView_imgui_Shutdown()` @0x801f0
  - Cleanup de ImGui

- `Java_com_mycompany_application_GLES3JNIView_resize()` @0x79fec
  - Responde a mudanças de viewport

#### Entrada de Usuário

- `Java_com_mycompany_application_GLES3JNIView_MotionEventClick()` @0x8022c
  - Processa eventos de toque/mouse

- `Java_com_mycompany_application_GLES3JNIView_getWindowRect()` @0x802f8
  - Obtém dimensões da janela para renderização

### Funcionalidades Identificadas

#### OpenGL ES

- Criação de shaders: `glCreateShader()`, `glShaderSource()`, `glCompileShader()`
- Linking: `glAttachShader()`, `glDetachShader()`, `glDeleteShader()`
- Queries: `glGetShaderiv()`, `glGetShaderInfoLog()`
- Suporta OpenGL ES 2.0 e 3.0

#### Framework C++

- STL C++ (libc++ NDK)
- Tratamento de exceções e erros
- String manipulation (std::string)
- Containers STL padrão

#### Compressão

- Dependência em libz.so indica processamento de dados comprimidos

#### Logging

- Dependência em liblog.so para logging via Android logcat

### Seções ELF Principais

```
.text                 - Código executável
.rodata              - Dados readonly (strings, constantes)
.data                - Dados inicializados
.bss                 - Dados não-inicializados (zerados)
.rela.dyn            - Relocações dinâmicas
.rela.plt            - Relocações de PLT
.dynsym              - Tabela de símbolos dinâmicos
.dynamic             - Informações dinâmicas de linking
.got                 - Global Offset Table
.got.plt             - GOT para PLT
```

---

## 3. Resumo da Aplicação

### Propósito

Aplicação Android com interface ImGui baseada em OpenGL ES 3.0, com suporte integrado a debugging remoto (ADRT).

### Arquitetura

```
┌─────────────────────────────────────┐
│     MainActivity (Java/Kotlin)      │
│  - Gerencia Activity lifecycle      │
│  - Trata eventos de toque          │
└────────────┬────────────────────────┘
             │
             ▼
┌─────────────────────────────────────┐
│     GLES3JNIView (Java)             │
│  - GLSurfaceView com OpenGL         │
│  - Chama métodos nativos via JNI   │
└────────────┬────────────────────────┘
             │
             ▼
┌─────────────────────────────────────┐
│   libmatveuxxx.so (C++ Nativo)      │
│  - ImGui rendering                 │
│  - OpenGL ES 3.0                   │
│  - Input handling                  │
│  - Frame management                │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│   ADRTSender & ADRTLogCatReader     │
│  - Debugging remoto                │
│  - Captura de logs em tempo real    │
└─────────────────────────────────────┘
```

### Fluxo de Execução

1. **Inicialização**:
   - MainActivity é criada
   - GLES3JNIView é adicionada ao layout
   - Carrega `libmatveuxxx.so`
   - `JNI_OnLoad()` registra callbacks

2. **Rendering Loop**:
   - `onSurfaceCreated()` → `init()` (nativo)
   - Cada frame chama `onDrawFrame()` → `step()` (nativo)
   - OpenGL ES renderiza ImGui

3. **Input**:
   - `onTouchEvent()` processa toque
   - Passa para `MotionEventClick()` nativo

4. **Debugging**:
   - ADRTSender envia eventos quando breakpoints/campos mudam
   - ADRTLogCatReader lê logs do sistema continuamente

### Uso de Memória

- **Code**: ~1.35 MB (libmatveuxxx.so)
- **Data**: ~1.05 MB (libmatveuxxx.so)
- **Dex**: ~1.3 MB (classes4.dex)
- **Total**: ~3.7 MB (aproximado)

### Dependências Externas

- Android SDK (API 21+, assumido)
- OpenGL ES 3.0 (na maioria dos dispositivos modernos)
- zlib (para compressão)

### Considerações de Segurança

- Aplicação com debugging remoto habilitado
- ADRT envia broadcasts sem restrição aparente
- Sem ofuscação evidente no bytecode DEX
- Biblioteca nativa is stripped (dificulta reversão manual)

---

## 4. Próximas Análises Recomendadas

### Com Ferramentas Avançadas

1. **Ghidra/IDA/Hopper**: Desmontar `libmatveuxxx.so` para análise detalhada
2. **JADX/Frida**: Análise dinâmica do DEX e código em execução
3. **Binary Ninja**: Análise de fluxo de controle da biblioteca nativa

### Focos de Investigação

- Implementação específica de ImGui nesta versão
- Detalhes do sistema ADRT para debugging remoto
- Strings/keys hardcoded na seção .rodata
- Padrões de comunicação com servidor de debugging
