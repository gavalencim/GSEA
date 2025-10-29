# Makefile para GSEA (Gestión Segura y Eficiente de Archivos)

# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS = 

# Nombre de los ejecutables
TARGET = gsea
INSPECTOR = gsea-inspect

# Archivos fuente
SOURCES = main.cpp
INSPECTOR_SOURCES = inspector.cpp
HEADERS = huffman.h aes-chiper.h

# Regla principal
all: $(TARGET) $(INSPECTOR)

# Compilar el programa principal
$(TARGET): $(SOURCES) $(HEADERS)
	@echo "Compilando GSEA..."
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	@echo "✓ Compilación exitosa: ./$(TARGET)"

# Compilar con información de debug
debug: CXXFLAGS += -g -DDEBUG
debug: clean $(TARGET)
	@echo "✓ Compilación en modo debug completada"

# Limpiar archivos compilados
clean:
	@echo "Limpiando archivos compilados..."
	rm -f $(TARGET) *.o
	@echo "✓ Limpieza completada"

# Instalar (copiar a /usr/local/bin)
install: $(TARGET)
	@echo "Instalando GSEA en /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "✓ Instalación completada"

# Desinstalar
uninstall:
	@echo "Desinstalando GSEA..."
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "✓ Desinstalación completada"

# Mostrar ayuda
help:
	@echo "Makefile para GSEA"
	@echo ""
	@echo "Uso:"
	@echo "  make          - Compilar el programa"
	@echo "  make debug    - Compilar con símbolos de debug"
	@echo "  make clean    - Limpiar archivos compilados"
	@echo "  make install  - Instalar en /usr/local/bin"
	@echo "  make uninstall- Desinstalar"
	@echo "  make test     - Ejecutar pruebas básicas"
	@echo "  make help     - Mostrar esta ayuda"

# Pruebas básicas
test: $(TARGET)
	@echo "Ejecutando pruebas básicas..."
	@echo ""
	@echo "=== Prueba 1: Comprimir un archivo ==="
	echo "Hola mundo desde GSEA" > test_input.txt
	./$(TARGET) -c -i test_input.txt -o test_compressed.huff
	@echo ""
	@echo "=== Prueba 2: Descomprimir ==="
	./$(TARGET) -d -i test_compressed.huff -o test_output.txt
	@echo ""
	@echo "=== Prueba 3: Verificar contenido ==="
	diff test_input.txt test_output.txt && echo "✓ Contenido verificado correctamente" || echo "✗ Error: contenidos diferentes"
	@echo ""
	@echo "=== Prueba 4: Comprimir y encriptar ==="
	./$(TARGET) -ce -i test_input.txt -o test_encrypted.gsea -k miClave123
	@echo ""
	@echo "=== Prueba 5: Desencriptar y descomprimir ==="
	./$(TARGET) -du -i test_encrypted.gsea -o test_final.txt -k miClave123
	@echo ""
	@echo "=== Verificación final ==="
	diff test_input.txt test_final.txt && echo "✓ TODAS LAS PRUEBAS PASARON" || echo "✗ Algunas pruebas fallaron"
	@echo ""
	@echo "Limpiando archivos de prueba..."
	rm -f test_input.txt test_compressed.huff test_output.txt test_encrypted.gsea test_final.txt
	@echo "✓ Pruebas completadas"

.PHONY: all debug clean install uninstall help test

