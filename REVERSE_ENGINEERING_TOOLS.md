# REA - Ferramentas de Engenharia Reversa

## ✅ Ferramentas Instaladas

### Depuração e Análise de Binários

#### GDB (GNU Debugger)
```bash
gdb <binary>                    # Iniciar debugger
gdb -x <script>                 # Executar script gdb
gdb --args <program> <args>     # Debugar programa com argumentos
```

#### Binutils
```bash
objdump -d <binary>             # Desassemblar seção .text
objdump -s <binary>             # Dump de seções
strings <binary>                # Extrair strings do binário
nm <binary>                     # Listar símbolos
readelf -a <binary>             # Análise completa de ELF
readelf -h <binary>             # Header ELF
readelf -l <binary>             # Program headers
```

#### Strace
```bash
strace <binary>                 # Rastrear system calls
strace -e trace=read <binary>   # Rastrear chamadas específicas
strace -o output.txt <binary>   # Salvar output
```

#### File Type Identification
```bash
file <binary>                   # Identificar tipo de arquivo
```

### Interpretação de Linguagens

#### Python3
```bash
python3 <script.py>             # Executar script Python
python3 -m <module>             # Executar módulo
```

#### Java
```bash
java -version                   # Verificar JVM
java -classpath <path> <class>  # Executar classe Java
```

---

## 🔧 Ferramentas para Instalar Manualmente

### 1. GHIDRA (NSA) - Disassembler/Decompiler Avançado

**Download:** https://github.com/NationalSecurityAgency/ghidra/releases

**Instalação:**
```bash
cd /opt/reverse-eng
unzip ghidra_PUBLIC.zip
./ghidra_*/ghidraRun          # Executar GUI
```

**Uso:**
- Análise de binários em múltiplas arquiteturas
- Decompilation de código
- Scripting com Python/Java
- Análise colaborativa

---

### 2. APKTool - Decompiler de APKs Android

**Download:** https://ibotpeaches.github.io/Apktool/

**Instalação:**
```bash
cd /opt/reverse-eng/bin
chmod +x apktool
./apktool --version
```

**Uso:**
```bash
apktool d <app.apk>                    # Decompile APK
apktool b <app-folder>                 # Build APK
apktool d -r <app.apk>                 # Sem decodificar recursos
```

---

### 3. Android SDK - Ferramentas Android

**Download:** https://developer.android.com/studio

**Instalação:**
```bash
tar -xzf android-commandlinetools-linux-*.tar.gz -C /opt
mkdir -p /opt/android-sdk
cd /opt/cmdline-tools && ./bin/sdkmanager --sdk_root=/opt/android-sdk --update
```

**Ferramentas Principais:**
```bash
adb                    # Android Debug Bridge
aapt                   # Android Asset Packaging Tool
dexdump                # DEX file analyzer
```

---

## 📊 Fluxo de Análise Típico

### Análise de Binário ELF/x86-64

```bash
# 1. Identificar arquivo
file myapp

# 2. Extrair strings
strings myapp | grep -i password

# 3. Analisar símbolos
nm -D myapp | grep main

# 4. Desassemblar com objdump
objdump -d myapp | head -50

# 5. Debugar com GDB
gdb myapp
(gdb) break main
(gdb) run
(gdb) disassemble
(gdb) print $rax

# 6. Rastrear chamadas de sistema
strace -e trace=file myapp
```

### Análise de APK Android

```bash
# 1. Decompile
apktool d app.apk

# 2. Analisar AndroidManifest.xml
cat app/AndroidManifest.xml

# 3. Decompile DEX
cd app/smali
# Analisar classes decompiladas

# 4. Extrair recursos
unzip -l app/resources.arsc

# 5. Usar apk analyzer
aapt dump badging app.apk
```

---

## 🎯 Integração com REA

REA (Reverse Engineer Anything) pode usar essas ferramentas para:

1. **Análise Automática**: Integrar `objdump`, `strings`, `readelf` como backends
2. **Android**: Combinar `apktool` com análise de `smali` decompilado
3. **Decompilation**: Usar `ghidra` scripting para análises programáticas
4. **Debugging**: Integrar `gdb` para análise dinâmica

### Exemplo com Python

```python
import subprocess
import json

def analyze_binary(path):
    # Identificar tipo
    file_info = subprocess.check_output(['file', path]).decode()
    
    # Extrair strings
    strings = subprocess.check_output(['strings', path]).decode()
    
    # Listar símbolos
    symbols = subprocess.check_output(['nm', path]).decode()
    
    return {
        'file_info': file_info,
        'strings': strings,
        'symbols': symbols
    }
```

---

## 📚 Recursos Adicionais

- **GDB Docs**: https://sourceware.org/gdb/documentation/
- **Ghidra**: https://github.com/NationalSecurityAgency/ghidra
- **APKTool**: https://ibotpeaches.github.io/Apktool/
- **Radare2**: https://rada.re/
- **Frida**: https://frida.re/ (Dynamic instrumentation)

---

**Last Updated**: 2026-10-17  
**Environment**: Ubuntu 24.04 LTS, OpenJDK 21
