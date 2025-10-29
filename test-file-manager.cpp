#include "file-manager.h"
#include <iostream>

int main() {
    std::cout << "=== Prueba del FileManager ===\n\n";
    
    // Prueba 1: Escribir un archivo
    std::cout << "1. Creando archivo de prueba...\n";
    std::vector<unsigned char> datos_test = {
        'H', 'o', 'l', 'a', ' ', 'm', 'u', 'n', 'd', 'o', '!', '\n'
    };
    
    FileManager escritor("test.txt");
    if (escritor.write_file(datos_test)) {
        std::cout << "   ✓ Archivo creado exitosamente\n\n";
    } else {
        std::cout << "   ✗ Error al crear archivo\n\n";
        return 1;
    }
    
    // Prueba 2: Leer el archivo
    std::cout << "2. Leyendo archivo...\n";
    FileManager lector("test.txt");
    std::vector<unsigned char> datos_leidos = lector.read_file();
    
    if (!datos_leidos.empty()) {
        std::cout << "   ✓ Archivo leído exitosamente\n";
        std::cout << "   Contenido: ";
        for (unsigned char c : datos_leidos) {
            std::cout << c;
        }
        std::cout << "\n\n";
    } else {
        std::cout << "   ✗ Error al leer archivo\n\n";
    }
    
    // Prueba 3: Verificar si un archivo existe
    std::cout << "3. Verificando existencia de archivos...\n";
    std::cout << "   test.txt existe: " 
              << (FileManager::file_exists("test.txt") ? "SÍ" : "NO") << "\n";
    std::cout << "   noexiste.txt existe: " 
              << (FileManager::file_exists("noexiste.txt") ? "SÍ" : "NO") << "\n\n";
    
    // Prueba 4: Verificar si es directorio
    std::cout << "4. Verificando directorios...\n";
    std::cout << "   '.' es directorio: " 
              << (FileManager::is_directory(".") ? "SÍ" : "NO") << "\n";
    std::cout << "   'test.txt' es directorio: " 
              << (FileManager::is_directory("test.txt") ? "SÍ" : "NO") << "\n\n";
    
    // Prueba 5: Listar archivos en el directorio actual
    std::cout << "5. Listando archivos en el directorio actual...\n";
    std::vector<std::string> archivos = FileManager::list_files(".");
    std::cout << "   Archivos encontrados: " << archivos.size() << "\n";
    for (const auto& archivo : archivos) {
        std::cout << "   - " << archivo << "\n";
    }
    
    std::cout << "\n=== Pruebas completadas ===\n";
    
    return 0;
}