#include "huffman.h"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== Prueba de Compresión Huffman ===\n\n";
    
    // Texto de ejemplo con repeticiones (ideal para Huffman)
    std::string text = "AAAAAABBBBBCCCCDDDEEF";
    std::cout << "Texto original: \"" << text << "\"\n";
    std::cout << "Tamaño original: " << text.length() << " bytes\n\n";
    
    // Convertir string a vector de unsigned char
    std::vector<unsigned char> input_data(text.begin(), text.end());
    
    // Crear el codificador Huffman
    HuffmanCoder huffman;
    
    // COMPRIMIR
    std::cout << "--- COMPRIMIENDO ---\n";
    std::vector<unsigned char> compressed = huffman.compress(input_data);
    std::cout << "\n";
    
    if (compressed.empty()) {
        std::cerr << "Error en la compresión\n";
        return 1;
    }
    
    // DESCOMPRIMIR
    std::cout << "--- DESCOMPRIMIENDO ---\n";
    std::vector<unsigned char> decompressed = huffman.decompress(compressed);
    std::cout << "\n";
    
    if (decompressed.empty()) {
        std::cerr << "Error en la descompresión\n";
        return 1;
    }
    
    // Convertir resultado a string para comparar
    std::string result(decompressed.begin(), decompressed.end());
    
    // VERIFICAR
    std::cout << "--- VERIFICACIÓN ---\n";
    std::cout << "Texto recuperado: \"" << result << "\"\n";
    std::cout << "Tamaño recuperado: " << result.length() << " bytes\n\n";
    
    if (text == result) {
        std::cout << "✓ ¡ÉXITO! Los datos fueron comprimidos y descomprimidos correctamente\n";
    } else {
        std::cout << "✗ ERROR: Los datos no coinciden\n";
        return 1;
    }
    
    // Calcular ratio de compresión
    double ratio = (100.0 * compressed.size()) / input_data.size();
    double saved = 100.0 - ratio;
    std::cout << "\nEstadísticas:\n";
    std::cout << "  Espacio ahorrado: " << saved << "%\n";
    std::cout << "  Factor de compresión: " << ((double)input_data.size() / compressed.size()) << "x\n";
    
    std::cout << "\n=== Prueba completada exitosamente ===\n";
    
    return 0;
}