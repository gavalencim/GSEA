# GSEA - Gestión Segura y Eficiente de Archivos

Sistema de compresión y encriptación de archivos implementado en C++ usando syscalls POSIX directas.

**Universidad EAFIT - Sistemas Operativos**

---

## 📋 Características

- ✅ **Compresión sin pérdida** con algoritmo Huffman implementado desde cero
- ✅ **Encriptación simétrica** con XOR mejorado (expansión de clave, S-box, encadenamiento)
- ✅ **Syscalls directas** de Linux (open, read, write, close, opendir, readdir)
- ✅ **Procesamiento de directorios** completos
- ✅ **Operaciones combinadas** (comprimir + encriptar en un solo comando)

---

## 🛠️ Compilación

### Opción 1: Usando Make
```bash
make clean
make
```

### Opción 2: Compilación manual
```bash
g++ -std=c++11 -Wall -O2 main.cpp -o gsea
```

### Requisitos
- g++ con soporte C++11
- Sistema operativo Linux/Unix (POSIX)

---

## 📖 Uso Básico

### Sintaxis General
```bash
./gsea [operaciones] -i <entrada> -o <salida> [-k <clave>]
```

### Operaciones Disponibles
| Opción | Descripción |
|--------|-------------|
| `-c` | Comprimir |
| `-d` | Descomprimir |
| `-e` | Encriptar |
| `-u` | Desencriptar (unlock) |
| `-k <clave>` | Clave secreta (obligatoria para encriptar/desencriptar) |

**Nota:** Las operaciones se pueden combinar (ej: `-ce` para comprimir y encriptar)

---

## 📚 Ejemplos de Uso

### 1️⃣ Comprimir un archivo
```bash
# Crear archivo de prueba
echo "Este es mi documento secreto" > documento.txt

# Comprimir
./gsea -c -i documento.txt -o documento.txt.huff

# Verificar
ls -lh documento.txt*
```

### 2️⃣ Descomprimir un archivo
```bash
./gsea -d -i documento.txt.huff -o documento_recuperado.txt

# Verificar que son idénticos
diff documento.txt documento_recuperado.txt
```

### 3️⃣ Encriptar un archivo
```bash
./gsea -e -i documento.txt -o documento.txt.enc -k miClaveSecreta123

# Ver contenido encriptado (ilegible)
hexdump -C documento.txt.enc | head
```

### 4️⃣ Desencriptar un archivo
```bash
./gsea -u -i documento.txt.enc -o documento_desencriptado.txt -k miClaveSecreta123

# Verificar
diff documento.txt documento_desencriptado.txt
```

### 5️⃣ Comprimir Y Encriptar (combinado)
```bash
# Orden: Primero comprime, luego encripta
./gsea -ce -i documento.txt -o documento.txt.gsea -k superSecreta
```

### 6️⃣ Desencriptar Y Descomprimir (orden inverso)
```bash
# Orden: Primero desencripta, luego descomprime
./gsea -du -i documento.txt.gsea -o documento_final.txt -k superSecreta

# Verificar
diff documento.txt documento_final.txt
```

---

## 🌍 Trabajar con Rutas

### Rutas Absolutas
```bash
# Comprimir archivo en ubicación específica
./gsea -c -i /home/usuario/Documents/informe.pdf -o /home/usuario/Documents/informe.pdf.huff

# Encriptar archivo en otra ubicación
./gsea -e -i /home/usuario/Downloads/foto.jpg -o /home/usuario/Downloads/foto.jpg.enc -k miClave
```

### Rutas Relativas
```bash
# Archivo en directorio padre
./gsea -c -i ../miarchivo.txt -o ../miarchivo.txt.huff

# Archivo en subcarpeta
./gsea -c -i ./datos/archivo.csv -o ./datos/archivo.csv.huff
```

### Usando ~ (Home Directory)
```bash
# Comprimir archivo en home
./gsea -c -i ~/Documents/trabajo.docx -o ~/Documents/trabajo.docx.huff

# Encriptar imagen
./gsea -e -i ~/Pictures/foto.jpg -o ~/Pictures/foto.jpg.enc -k password123
```

### En WSL (Windows Subsystem for Linux)
```bash
# Acceder a archivos de Windows desde WSL
./gsea -c -i /mnt/c/Users/Usuario/Documents/archivo.txt -o /mnt/c/Users/Usuario/Documents/archivo.txt.huff

# Escritorio de Windows
./gsea -e -i /mnt/c/Users/Usuario/Desktop/secreto.pdf -o /mnt/c/Users/Usuario/Desktop/secreto.pdf.enc -k clave123
```

---

## 📁 Procesar Directorios Completos

### Comprimir todos los archivos de una carpeta
```bash
# Crear estructura de prueba
mkdir documentos
echo "Archivo 1" > documentos/file1.txt
echo "Archivo 2" > documentos/file2.txt
echo "Archivo 3" > documentos/file3.txt

# Crear carpeta de salida
mkdir documentos_comprimidos

# Comprimir todos los archivos
./gsea -c -i documentos -o documentos_comprimidos

# Ver resultados
ls -lh documentos_comprimidos/
```

**Salida esperada:**
```
file1.txt.huff
file2.txt.huff
file3.txt.huff
```

### Encriptar todos los archivos de una carpeta
```bash
mkdir documentos_encriptados
./gsea -e -i documentos -o documentos_encriptados -k miClave123

# Ver resultados
ls -lh documentos_encriptados/
```

---

## 🎯 Casos de Uso Prácticos

### Backup de Documentos Importantes
```bash
# Comprimir y encriptar carpeta completa
./gsea -ce -i ~/Documents/importantes -o ~/Backups/importantes_seguros -k ClaveSegura2024

# Para recuperar:
./gsea -du -i ~/Backups/importantes_seguros -o ~/Documents/recuperados -k ClaveSegura2024
```

### Comprimir Archivos Grandes
```bash
# Comprimir PDF grande
./gsea -c -i ~/Downloads/manual_1GB.pdf -o ~/Downloads/manual_1GB.pdf.huff

# Comparar tamaños
ls -lh ~/Downloads/manual_1GB*
```

### Enviar Archivos Confidenciales
```bash
# Encriptar antes de enviar por email
./gsea -e -i ~/Documents/contrato.pdf -o ~/Desktop/contrato.enc -k ClaveCompartida

# El destinatario desencripta con la misma clave:
./gsea -u -i contrato.enc -o contrato.pdf -k ClaveCompartida
```

### Proteger Fotografías Privadas
```bash
# Encriptar carpeta de fotos
./gsea -e -i ~/Pictures/privadas -o ~/Pictures/privadas_enc -k MiClavePrivada
```

---

## 🔍 Verificación y Testing

### Verificar que un archivo se procesó correctamente
```bash
# 1. Comprimir
./gsea -c -i original.txt -o comprimido.huff

# 2. Descomprimir
./gsea -d -i comprimido.huff -o recuperado.txt

# 3. Comparar (no imprime nada si son idénticos)
diff original.txt recuperado.txt

# 4. Comparar tamaños
ls -lh original.txt comprimido.huff recuperado.txt
```

### Script de Prueba Rápida

Crea un archivo `quick_test.sh`:
```bash
#!/bin/bash

echo "=== GSEA Quick Test ==="

# Crear archivo de prueba
echo "Contenido de prueba para comprimir y encriptar" > test.txt
echo "✓ Archivo creado: test.txt"

# Comprimir
./gsea -c -i test.txt -o test.huff
echo "✓ Comprimido: test.huff"

# Descomprimir
./gsea -d -i test.huff -o test_recovered.txt
echo "✓ Descomprimido: test_recovered.txt"

# Verificar
if diff test.txt test_recovered.txt > /dev/null; then
    echo "✓ ¡ÉXITO! Los archivos son idénticos"
else
    echo "✗ ERROR: Los archivos son diferentes"
fi

# Mostrar tamaños
echo ""
echo "Tamaños:"
ls -lh test.txt test.huff test_recovered.txt

# Limpiar
rm -f test.txt test.huff test_recovered.txt
echo ""
echo "✓ Archivos de prueba eliminados"
```

Ejecutar:
```bash
chmod +x quick_test.sh
./quick_test.sh
```

---

## 🚨 Errores Comunes y Soluciones

### Error: "No such file or directory"
```bash
# ✗ Incorrecto (archivo no existe)
./gsea -c -i noexiste.txt -o salida.huff

# ✓ Correcto (verificar primero)
ls archivo.txt
./gsea -c -i archivo.txt -o salida.huff
```

### Error: "Clave requerida"
```bash
# ✗ Incorrecto (falta -k)
./gsea -e -i archivo.txt -o archivo.enc

# ✓ Correcto
./gsea -e -i archivo.txt -o archivo.enc -k miClave123
```

### Error: Clave incorrecta al desencriptar
```bash
# Encriptar con una clave
./gsea -e -i archivo.txt -o archivo.enc -k abc123

# ✗ Incorrecto (clave diferente)
./gsea -u -i archivo.enc -o salida.txt -k xyz789
# Resultado: datos corruptos

# ✓ Correcto (misma clave)
./gsea -u -i archivo.enc -o salida.txt -k abc123
```

---

## 🏗️ Arquitectura del Sistema

### Componentes Principales

1. **main.cpp**: Programa principal con funciones de I/O usando syscalls directas
2. **huffman.h**: Implementación del algoritmo de compresión Huffman
3. **xor_cipher.h**: Implementación del cifrado XOR mejorado

### Syscalls Utilizadas

| Syscall | Propósito |
|---------|-----------|
| `open()` | Abrir archivos |
| `read()` | Leer datos del archivo |
| `write()` | Escribir datos al archivo |
| `close()` | Cerrar file descriptors |
| `stat()` / `fstat()` | Obtener información del archivo |
| `opendir()` | Abrir directorio |
| `readdir()` | Leer entradas del directorio |
| `closedir()` | Cerrar directorio |

### Flujo de Operaciones

#### Comprimir + Encriptar
```
Archivo Original → [Huffman Compress] → Datos Comprimidos → [XOR Encrypt] → Archivo Final
```

#### Desencriptar + Descomprimir
```
Archivo Encriptado → [XOR Decrypt] → Datos Comprimidos → [Huffman Decompress] → Archivo Original
```

---

## 📊 Algoritmos Implementados

### Compresión: Huffman Coding

- **Tipo**: Sin pérdida (lossless)
- **Estrategia**: Codificación de longitud variable basada en frecuencias
- **Implementación**: Desde cero (sin librerías externas)
- **Características**:
  - Construcción de árbol binario óptimo
  - Serialización del árbol en el archivo comprimido
  - Códigos más cortos para símbolos más frecuentes

### Encriptación: XOR Mejorado

- **Tipo**: Cifrado simétrico
- **Mejoras sobre XOR básico**:
  - Expansión de clave mediante función hash (256 bytes)
  - S-box para sustitución no lineal (inspirado en AES)
  - Encadenamiento de bloques (cada byte depende del anterior)
  - Rotación de bits dependiente del estado
  - Difusión mediante XOR con posición

---

## 📝 Estructura de Archivos
```
proyecto3/
├── main.cpp              # Programa principal con syscalls
├── huffman.h             # Algoritmo de compresión Huffman
├── xor.h          # Algoritmo de encriptación XOR
├── Makefile              # Script de compilación
├── README.md             # Este archivo
└── test-gsea.sh          # Script de pruebas automatizado (opcional)
```

---

## 🧪 Testing

### Ejecutar Suite de Pruebas Completa

Si tienes el script `test-gsea.sh`:
```bash
chmod +x test-gsea.sh
./test-gsea.sh
```

El script prueba:
- ✅ Compresión y descompresión
- ✅ Encriptación y desencriptación
- ✅ Operaciones combinadas
- ✅ Integridad de datos
- ✅ Verificación de tamaños

---

## ⚠️ Consideraciones de Seguridad

1. **Fortaleza del cifrado**: El algoritmo XOR mejorado es educativo. Para datos críticos, usar AES-256.
2. **Gestión de claves**: Las claves se pasan por línea de comandos (visibles en historial). En producción, usar métodos más seguros.
3. **No hay autenticación**: El cifrado no incluye MAC/HMAC para verificar integridad.
4. **Claves fuertes**: Usar claves largas y complejas (mínimo 16 caracteres).

---

## 📄 Licencia

Proyecto académico - Universidad EAFIT  
Sistemas Operativos - 2025-2

---

## 👥 Autor

[Ginna Alejandra]  
[gavalencim@eafit.edu.co/GitHub]

---

## 📚 Referencias

- Arpaci-Dusseau, R. H., & Arpaci-Dusseau, A. C. (2018). *Operating Systems: Three Easy Pieces*
- Huffman, D. A. (1952). *A Method for the Construction of Minimum-Redundancy Codes*
- POSIX.1-2017 System Interfaces

---

## 🆘 Soporte

Si encuentras problemas:

1. Verifica que tienes permisos de lectura/escritura en los directorios
2. Asegúrate de usar la misma clave para encriptar y desencriptar
3. Comprueba que los archivos de entrada existen
4. Revisa los mensajes de error en la consola

Para más ayuda, consulta la documentación del curso o contacta al instructor.
