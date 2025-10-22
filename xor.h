#ifndef XOR_CIPHER_H
#define XOR_CIPHER_H

#include <vector>
#include <string>
#include <iostream>
#include <cstdint>

// Clase para encriptación/desencriptación XOR mejorada
class XORCipher {
private:
    std::string key;                    // Clave secreta del usuario
    std::vector<unsigned char> expanded_key;  // Clave expandida
    static const int KEY_EXPANSION_SIZE = 256;  // Tamaño de la clave expandida
    
    // Función hash simple para expandir la clave
    // Convierte una clave de cualquier longitud en una de 256 bytes
    void expand_key() {
        expanded_key.clear();
        expanded_key.resize(KEY_EXPANSION_SIZE);
        
        if (key.empty()) {
            std::cerr << "Error: Clave vacía\n";
            return;
        }
        
        // Usar un algoritmo de hash simple (inspirado en DJB2)
        uint32_t hash = 5381;
        
        for (int i = 0; i < KEY_EXPANSION_SIZE; i++) {
            // Mezclar el índice actual con la clave
            for (size_t j = 0; j < key.length(); j++) {
                // DJB2 hash: hash = hash * 33 + c
                hash = ((hash << 5) + hash) + key[j] + i;
            }
            
            // Tomar el byte menos significativo del hash
            expanded_key[i] = hash & 0xFF;
            
            // Rotar el hash para el siguiente byte
            hash = (hash >> 8) | (hash << 24);
        }
        
        std::cout << "  → Clave expandida a " << KEY_EXPANSION_SIZE << " bytes\n";
    }
    
    // Función auxiliar para rotar bits a la izquierda
    unsigned char rotate_left(unsigned char value, int positions) {
        positions = positions % 8;  // Asegurar que positions esté en rango 0-7
        return (value << positions) | (value >> (8 - positions));
    }
    
    // Función auxiliar para rotar bits a la derecha
    unsigned char rotate_right(unsigned char value, int positions) {
        positions = positions % 8;
        return (value >> positions) | (value << (8 - positions));
    }
    
    // Aplicar transformación no lineal (S-box simplificada)
    // Esto dificulta el análisis de frecuencias
    unsigned char apply_sbox(unsigned char value) {
        // Tabla de sustitución simple (S-box)
        // En un cifrado real como AES, esta tabla está cuidadosamente diseñada
        static const unsigned char sbox[256] = {
            0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
            0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
            0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
            0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
            0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
            0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
            0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
            0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
            0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
            0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
            0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
            0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
            0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
            0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
            0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
            0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
        };
        
        return sbox[value];
    }
    
    // Aplicar transformación inversa (S-box inversa)
    unsigned char apply_inverse_sbox(unsigned char value) {
        // Tabla de sustitución inversa
        static const unsigned char inv_sbox[256] = {
            0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
            0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
            0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
            0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
            0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
            0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
            0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
            0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
            0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
            0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
            0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
            0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
            0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
            0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
            0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
            0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
        };
        
        return inv_sbox[value];
    }
    
public:
    // Constructor
    XORCipher(const std::string& user_key) : key(user_key) {
        expand_key();
    }
    
    // ENCRIPTAR: Aplica XOR mejorado con modificación de estado
    std::vector<unsigned char> encrypt(const std::vector<unsigned char>& plaintext) {
        if (plaintext.empty()) {
            std::cerr << "Error: Datos vacíos para encriptar\n";
            return std::vector<unsigned char>();
        }
        
        if (expanded_key.empty()) {
            std::cerr << "Error: Clave no inicializada\n";
            return std::vector<unsigned char>();
        }
        
        std::vector<unsigned char> ciphertext;
        ciphertext.reserve(plaintext.size());
        
        // Estado inicial basado en la suma de la clave expandida
        unsigned char state = 0;
        for (unsigned char k : expanded_key) {
            state ^= k;
        }
        
        std::cout << "  → Encriptando " << plaintext.size() << " bytes...\n";
        
        // Procesar cada byte del texto plano
        for (size_t i = 0; i < plaintext.size(); i++) {
            unsigned char plain_byte = plaintext[i];
            
            // PASO 1: XOR con la clave expandida (rotada según posición)
            unsigned char key_byte = expanded_key[i % KEY_EXPANSION_SIZE];
            unsigned char temp = plain_byte ^ key_byte;
            
            // PASO 2: Aplicar S-box (sustitución no lineal)
            temp = apply_sbox(temp);
            
            // PASO 3: XOR con el estado actual (encadenamiento)
            // Esto hace que cada byte dependa del anterior
            temp ^= state;
            
            // PASO 4: Rotar bits según el estado
            temp = rotate_left(temp, state % 8);
            
            // PASO 5: XOR adicional con posición (difusión)
            temp ^= (i & 0xFF);
            
            // Guardar el byte encriptado
            ciphertext.push_back(temp);
            
            // PASO 6: Actualizar el estado para el siguiente byte
            // El estado se mezcla con el byte encriptado y la posición
            state = (state + temp + key_byte) & 0xFF;
        }
        
        std::cout << "  → Encriptación completada\n";
        
        return ciphertext;
    }
    
    // DESENCRIPTAR: Proceso inverso de la encriptación
    std::vector<unsigned char> decrypt(const std::vector<unsigned char>& ciphertext) {
        if (ciphertext.empty()) {
            std::cerr << "Error: Datos vacíos para desencriptar\n";
            return std::vector<unsigned char>();
        }
        
        if (expanded_key.empty()) {
            std::cerr << "Error: Clave no inicializada\n";
            return std::vector<unsigned char>();
        }
        
        std::vector<unsigned char> plaintext;
        plaintext.reserve(ciphertext.size());
        
        // Estado inicial (mismo que en encriptación)
        unsigned char state = 0;
        for (unsigned char k : expanded_key) {
            state ^= k;
        }
        
        std::cout << "  → Desencriptando " << ciphertext.size() << " bytes...\n";
        
        // Procesar cada byte del texto cifrado (en orden inverso a la encriptación)
        for (size_t i = 0; i < ciphertext.size(); i++) {
            unsigned char cipher_byte = ciphertext[i];
            
            // Obtener la clave para esta posición
            unsigned char key_byte = expanded_key[i % KEY_EXPANSION_SIZE];
            
            // PASO 1 (inverso): XOR con posición
            unsigned char temp = cipher_byte ^ (i & 0xFF);
            
            // PASO 2 (inverso): Rotar bits a la derecha
            temp = rotate_right(temp, state % 8);
            
            // PASO 3 (inverso): XOR con el estado
            temp ^= state;
            
            // PASO 4 (inverso): Aplicar S-box inversa
            temp = apply_inverse_sbox(temp);
            
            // PASO 5 (inverso): XOR con la clave expandida
            unsigned char plain_byte = temp ^ key_byte;
            
            // Guardar el byte desencriptado
            plaintext.push_back(plain_byte);
            
            // PASO 6: Actualizar el estado (igual que en encriptación)
            // IMPORTANTE: Usamos cipher_byte (no plain_byte) porque es lo que teníamos en encrypt
            state = (state + cipher_byte + key_byte) & 0xFF;
        }
        
        std::cout << "  → Desencriptación completada\n";
        
        return plaintext;
    }
    
    // Función auxiliar para mostrar bytes en hexadecimal (útil para debug)
    static void print_hex(const std::vector<unsigned char>& data, int max_bytes = 16) {
        for (size_t i = 0; i < data.size() && i < (size_t)max_bytes; i++) {
            printf("%02x ", data[i]);
        }
        if (data.size() > (size_t)max_bytes) {
            std::cout << "...";
        }
        std::cout << "\n";
    }
};

#endif // XOR_CIPHER_H