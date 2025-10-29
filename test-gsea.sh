#!/bin/bash

# Script de prueba para GSEA
# Verifica que todo funcione correctamente

echo "╔════════════════════════════════════════════════════════╗"
echo "║  GSEA - Script de Pruebas Completo                   ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# Colores para output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Función para verificar si un archivo existe
check_file() {
    if [ -f "$1" ]; then
        echo -e "${GREEN}✓${NC} Archivo existe: $1 ($(ls -lh "$1" | awk '{print $5}'))"
        return 0
    else
        echo -e "${RED}✗${NC} Archivo NO existe: $1"
        return 1
    fi
}

# Limpiar archivos de pruebas anteriores
echo "→ Limpiando archivos de pruebas anteriores..."
rm -f test_*.txt test_*.huff test_*.enc test_*.gsea 2>/dev/null
echo ""

# Verificar que el ejecutable existe
if [ ! -f "./gsea" ]; then
    echo -e "${RED}✗ Error: ./gsea no existe. Compilar primero con 'make'${NC}"
    exit 1
fi

echo -e "${GREEN}✓${NC} Ejecutable gsea encontrado"
echo ""

# ============================================================================
# PRUEBA 1: Comprimir un archivo
# ============================================================================
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "PRUEBA 1: Comprimir archivo"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Crear archivo de prueba
echo "Hola mundo desde GSEA. Este es un mensaje de prueba con repeticiones AAAA BBBB CCCC" > test_original.txt
echo -e "${GREEN}✓${NC} Archivo de prueba creado"
check_file "test_original.txt"
echo ""

# Comprimir
echo "→ Ejecutando: ./gsea -c -i test_original.txt -o test_compressed.huff"
./gsea -c -i test_original.txt -o test_compressed.huff
echo ""

# Verificar que se creó
if check_file "test_compressed.huff"; then
    echo -e "${GREEN}✓ PRUEBA 1 EXITOSA${NC}"
else
    echo -e "${RED}✗ PRUEBA 1 FALLÓ${NC}"
    exit 1
fi
echo ""

# ============================================================================
# PRUEBA 2: Descomprimir el archivo
# ============================================================================
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "PRUEBA 2: Descomprimir archivo"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo "→ Ejecutando: ./gsea -d -i test_compressed.huff -o test_decompressed.txt"
./gsea -d -i test_compressed.huff -o test_decompressed.txt
echo ""

if check_file "test_decompressed.txt"; then
    # Comparar contenidos
    if diff test_original.txt test_decompressed.txt > /dev/null 2>&1; then
        echo -e "${GREEN}✓ Los archivos son IDÉNTICOS${NC}"
        echo -e "${GREEN}✓ PRUEBA 2 EXITOSA${NC}"
    else
        echo -e "${RED}✗ Los archivos son DIFERENTES${NC}"
        echo "Contenido original:"
        cat test_original.txt
        echo ""
        echo "Contenido descomprimido:"
        cat test_decompressed.txt
        exit 1
    fi
else
    echo -e "${RED}✗ PRUEBA 2 FALLÓ${NC}"
    exit 1
fi
echo ""

# ============================================================================
# PRUEBA 3: Encriptar un archivo
# ============================================================================
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "PRUEBA 3: Encriptar archivo"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo "→ Ejecutando: ./gsea -e -i test_original.txt -o test_encrypted.enc -k miClave123"
./gsea -e -i test_original.txt -o test_encrypted.enc -k miClave123
echo ""

if check_file "test_encrypted.enc"; then
    echo -e "${GREEN}✓ PRUEBA 3 EXITOSA${NC}"
    
    # Verificar que el contenido es diferente
    echo ""
    echo "→ Comparando archivos..."
    echo "  Original (primeros 50 chars):"
    head -c 50 test_original.txt
    echo ""
    echo "  Encriptado (hex):"
    hexdump -C test_encrypted.enc | head -3
else
    echo -e "${RED}✗ PRUEBA 3 FALLÓ${NC}"
    exit 1
fi
echo ""

# ============================================================================
# PRUEBA 4: Desencriptar el archivo
# ============================================================================
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "PRUEBA 4: Desencriptar archivo"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo "→ Ejecutando: ./gsea -u -i test_encrypted.enc -o test_decrypted.txt -k miClave123"
./gsea -u -i test_encrypted.enc -o test_decrypted.txt -k miClave123
echo ""

if check_file "test_decrypted.txt"; then
    if diff test_original.txt test_decrypted.txt > /dev/null 2>&1; then
        echo -e "${GREEN}✓ Los archivos son IDÉNTICOS${NC}"
        echo -e "${GREEN}✓ PRUEBA 4 EXITOSA${NC}"
    else
        echo -e "${RED}✗ Los archivos son DIFERENTES${NC}"
        exit 1
    fi
else
    echo -e "${RED}✗ PRUEBA 4 FALLÓ${NC}"
    exit 1
fi
echo ""

# ============================================================================
# PRUEBA 5: Comprimir Y Encriptar
# ============================================================================
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "PRUEBA 5: Comprimir Y Encriptar (combinado)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo "→ Ejecutando: ./gsea -ce -i test_original.txt -o test_both.gsea -k password"
./gsea -ce -i test_original.txt -o test_both.gsea -k password
echo ""

if check_file "test_both.gsea"; then
    echo -e "${GREEN}✓ PRUEBA 5 EXITOSA${NC}"
else
    echo -e "${RED}✗ PRUEBA 5 FALLÓ${NC}"
    exit 1
fi
echo ""

# ============================================================================
# PRUEBA 6: Desencriptar Y Descomprimir
# ============================================================================
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "PRUEBA 6: Desencriptar Y Descomprimir (orden inverso)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo "→ Ejecutando: ./gsea -du -i test_both.gsea -o test_recovered.txt -k password"
./gsea -du -i test_both.gsea -o test_recovered.txt -k password
echo ""

if check_file "test_recovered.txt"; then
    if diff test_original.txt test_recovered.txt > /dev/null 2>&1; then
        echo -e "${GREEN}✓ Los archivos son IDÉNTICOS${NC}"
        echo -e "${GREEN}✓ PRUEBA 6 EXITOSA${NC}"
    else
        echo -e "${RED}✗ Los archivos son DIFERENTES${NC}"
        exit 1
    fi
else
    echo -e "${RED}✗ PRUEBA 6 FALLÓ${NC}"
    exit 1
fi
echo ""

# ============================================================================
# RESUMEN
# ============================================================================
echo "╔════════════════════════════════════════════════════════╗"
echo "║  ✓ TODAS LAS PRUEBAS PASARON EXITOSAMENTE            ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

echo "Archivos generados:"
ls -lh test_*.txt test_*.huff test_*.enc test_*.gsea 2>/dev/null
echo ""

echo "¿Deseas limpiar los archivos de prueba? (y/n)"
read -r response
if [[ "$response" =~ ^[Yy]$ ]]; then
    rm -f test_*.txt test_*.huff test_*.enc test_*.gsea
    echo "✓ Archivos de prueba eliminados"
else
    echo "→ Archivos de prueba conservados para inspección"
fi