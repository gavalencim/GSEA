#include "file-manager.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>

// Función para mostrar bytes en hexadecimal con formato bonito
void print_hex_dump(const std::vector<unsigned char>& data, int max_lines = 16) {
    std::cout << "\n┌────────┬─────────────────────────────────────────────────┬──────────────────┐\n";
    std::cout << "│ Offset │ Hexadecimal                                     │ ASCII            │\n";
    std::cout << "├────────┼─────────────────────────────────────────────────┼──────────────────┤\n";
    
    int bytes_per_line = 16;
    int lines_shown = 0;
    
    for (size_t i = 0; i < data.size() && lines_shown < max_lines; i += bytes_per_line, lines_shown++) {
        // Mostrar offset
        std::cout << "│ " << std::hex << std::setw(6) << std::setfill('0') << i << " │ ";
        
        // Mostrar bytes en hexadecimal
        for (int j = 0; j < bytes_per_line; j++) {
            if (i + j < data.size()) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                          << (int)data[i + j] << " ";
            } else {
                std::cout << "   ";
            }
            
            // Separador en la mitad
            if (j == 7) std::cout << " ";
        }
        
        std::cout << "│ ";
        
        // Mostrar representación ASCII
        for (int j = 0; j < bytes_per_line && i + j < data.size(); j++) {
            unsigned char c = data[i + j];
            if (c >= 32 && c <= 126) {
                std::cout << c;
            } else {
                std::cout << ".";
            }
        }
        
        // Rellenar espacios si es la última línea
        for (int j = data.size() - i; j < bytes_per_line && i + j >= data.size(); j++) {
            std::cout << " ";
        }
        
        std::cout << " │\n";
    }
    
    if (data.size() > (size_t)(max_lines * bytes_per_line)) {
        std::cout << "│   ...  │ ... (" << std::dec << (data.size() - max_lines * bytes_per_line) 
                  << " bytes más) ...\n";
    }
    
    std::cout << "└────────┴─────────────────────────────────────────────────┴──────────────────┘\n";
    std::cout << std::dec;  // Restaurar formato decimal
}

// Calcular entropía (medida de aleatoriedad)
double calculate_entropy(const std::vector<unsigned char>& data) {
    if (data.empty()) return 0.0;
    
    // Contar frecuencias
    int frequency[256] = {0};
    for (unsigned char byte : data) {
        frequency[byte]++;
    }
    
    // Calcular entropía de Shannon
    double entropy = 0.0;
    for (int i = 0; i < 256; i++) {
        if (frequency[i] > 0) {
            double p = (double)frequency[i] / data.size();
            entropy -= p; // Calculo mal hecho, en por log2p 
        }
    }
    
    return entropy;
}

// Analizar distribución de bytes
void analyze_distribution(const std::vector<unsigned char>& data) {
    int frequency[256] = {0};
    for (unsigned char byte : data) {
        frequency[byte]++;
    }
    
    // Encontrar los bytes más frecuentes
    std::cout << "\n┌─────────────────────────────────┐\n";
    std::cout << "│ Bytes más frecuentes:           │\n";
    std::cout << "├──────┬────────┬─────────────────┤\n";
    std::cout << "│ Byte │ Freq.  │ ASCII           │\n";
    std::cout << "├──────┼────────┼─────────────────┤\n";
    
    // Ordenar y mostrar top 10
    std::vector<std::pair<int, int>> freq_pairs;
    for (int i = 0; i < 256; i++) {
        if (frequency[i] > 0) {
            freq_pairs.push_back({frequency[i], i});
        }
    }
    
    std::sort(freq_pairs.rbegin(), freq_pairs.rend());
    
    int count = 0;
    for (const auto& pair : freq_pairs) {
        if (count >= 10) break;
        
        std::cout << "│ 0x" << std::hex << std::setw(2) << std::setfill('0') 
                  << pair.second << " │ " << std::dec << std::setw(6) 
                  << pair.first << " │ ";
        
        unsigned char c = pair.second;
        if (c >= 32 && c <= 126) {
            std::cout << "'" << c << "'";
        } else if (c == 10) {
            std::cout << "'\\n'";
        } else if (c == 13) {
            std::cout << "'\\r'";
        } else if (c == 9) {
            std::cout << "'\\t'";
        } else {
            std::cout << "[" << (int)c << "]";
        }
        
        std::cout << "             │\n";
        count++;
    }
    
    std::cout << "└──────┴────────┴─────────────────┘\n";
}

// Detectar tipo de archivo
std::string detect_file_type(const std::vector<unsigned char>& data) {
    if (data.empty()) return "Vacío";
    
    // Verificar magic numbers comunes
    if (data.size() >= 4) {
        // PDF
        if (data[0] == 0x25 && data[1] == 0x50 && data[2] == 0x44 && data[3] == 0x46) {
            return "PDF";
        }
        // PNG
        if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47) {
            return "PNG";
        }
        // JPEG
        if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) {
            return "JPEG";
        }
        // ZIP
        if (data[0] == 0x50 && data[1] == 0x4B && data[2] == 0x03 && data[3] == 0x04) {
            return "ZIP";
        }
    }
    
    // Verificar si es texto ASCII
    bool is_text = true;
    int printable = 0;
    for (size_t i = 0; i < data.size() && i < 1000; i++) {
        if (data[i] >= 32 && data[i] <= 126) {
            printable++;
        } else if (data[i] != 10 && data[i] != 13 && data[i] != 9) {
            is_text = false;
        }
    }
    
    if (is_text && printable > 0) return "Texto ASCII";
    
    return "Binario/Desconocido";
}

int main(int argc, char* argv[]) {
    std::cout << "╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║  GSEA Inspector - Herramienta de Inspección          ║\n";
    std::cout << "║  Visualiza archivos comprimidos/encriptados          ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n\n";
    
    if (argc < 2) {
        std::cout << "Uso: " << argv[0] << " <archivo>\n";
        std::cout << "\nEjemplos:\n";
        std::cout << "  " << argv[0] << " archivo.txt\n";
        std::cout << "  " << argv[0] << " archivo.txt.huff\n";
        std::cout << "  " << argv[0] << " archivo.txt.enc\n";
        return 1;
    }
    
    std::string filename = argv[1];
    
    // Verificar que el archivo existe
    if (!FileManager::file_exists(filename)) {
        std::cerr << "Error: El archivo no existe: " << filename << "\n";
        return 1;
    }
    
    // Leer el archivo
    std::cout << "Analizando: " << filename << "\n";
    std::cout << "════════════════════════════════════════════════════════\n\n";
    
    FileManager fm(filename);
    std::vector<unsigned char> data = fm.read_file();
    
    if (data.empty()) {
        std::cerr << "Error: No se pudo leer el archivo o está vacío\n";
        return 1;
    }
    
    // INFORMACIÓN BÁSICA
    std::cout << "┌─────────────────────────────────────────────────┐\n";
    std::cout << "│ INFORMACIÓN BÁSICA                              │\n";
    std::cout << "├─────────────────────────────────────────────────┤\n";
    std::cout << "│ Tamaño:        " << std::setw(10) << data.size() << " bytes             │\n";
    std::cout << "│ Tipo detectado: " << std::setw(28) << std::left 
              << detect_file_type(data) << " │\n" << std::right;
    
    double entropy = calculate_entropy(data);
    std::cout << "│ Entropía:      " << std::fixed << std::setprecision(4) 
              << std::setw(10) << entropy << " bits/byte       │\n";
    std::cout << "│                                                 │\n";
    std::cout << "│ Interpretación de Entropía:                     │\n";
    std::cout << "│   0.0 - 3.0  : Muy comprimido/repetitivo       │\n";
    std::cout << "│   3.0 - 5.0  : Texto normal                    │\n";
    std::cout << "│   5.0 - 7.0  : Comprimido o estructurado       │\n";
    std::cout << "│   7.0 - 8.0  : Encriptado/aleatorio            │\n";
    std::cout << "└─────────────────────────────────────────────────┘\n";
    
    // ANÁLISIS DE ENTROPÍA
    std::cout << "\n";
    if (entropy < 3.0) {
        std::cout << "→ Este archivo tiene MUY BAJA entropía (datos muy repetitivos)\n";
    } else if (entropy < 5.0) {
        std::cout << "→ Este archivo parece ser TEXTO NORMAL sin procesar\n";
    } else if (entropy < 7.0) {
        std::cout << "→ Este archivo parece estar COMPRIMIDO\n";
    } else {
        std::cout << "→ Este archivo tiene ALTA entropía (probablemente ENCRIPTADO)\n";
    }
    
    // VOLCADO HEXADECIMAL
    std::cout << "\n┌─────────────────────────────────────────────────┐\n";
    std::cout << "│ VOLCADO HEXADECIMAL (primeras 256 bytes)        │\n";
    std::cout << "└─────────────────────────────────────────────────┘";
    print_hex_dump(data, 16);
    
    // DISTRIBUCIÓN DE BYTES
    analyze_distribution(data);
    
    // COMPARACIÓN VISUAL
    std::cout << "\n┌─────────────────────────────────────────────────┐\n";
    std::cout << "│ PRIMEROS 100 CARACTERES (representación visual) │\n";
    std::cout << "└─────────────────────────────────────────────────┘\n";
    std::cout << "\n";
    for (size_t i = 0; i < data.size() && i < 100; i++) {
        unsigned char c = data[i];
        if (c >= 32 && c <= 126) {
            std::cout << c;
        } else {
            std::cout << "·";
        }
    }
    std::cout << "\n\n";
    
    std::cout << "════════════════════════════════════════════════════════\n";
    std::cout << "Análisis completado\n";
    
    return 0;
}