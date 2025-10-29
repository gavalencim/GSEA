#ifndef AES_CIPHER_H
#define AES_CIPHER_H

#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <cstdint>

/**
 * AES SIMPLIFICADO (Educativo)
 * 
 * Implementación simplificada de AES (Advanced Encryption Standard)
 * Usa bloques de 16 bytes (128 bits) y clave de 16 bytes (128 bits)
 * 
 * Operaciones implementadas:
 * 1. SubBytes: Sustitución no lineal usando S-box
 * 2. ShiftRows: Permutación de filas
 * 3. MixColumns: Mezcla de columnas (simplificado)
 * 4. AddRoundKey: XOR con clave de ronda
 * 
 * Nota: Esta es una versión educativa simplificada.
 * Para producción usar librerías criptográficas estándar.
 */

class AESCipher {
private:
    static const int BLOCK_SIZE = 16;  // 128 bits = 16 bytes
    static const int KEY_SIZE = 16;    // 128 bits = 16 bytes
    static const int NUM_ROUNDS = 4;   // Versión simplificada (AES real usa 10)
    
    std::vector<uint8_t> key_schedule;  // Claves expandidas para cada ronda
    
    // ========================================================================
    // S-BOX de AES (Tabla de sustitución estándar de AES)
    // ========================================================================
    static const uint8_t SBOX[256];
    static const uint8_t INV_SBOX[256];
    
    // ========================================================================
    // FUNCIONES AUXILIARES
    // ========================================================================
    
    /**
     * Rota los bits de un byte hacia la izquierda
     */
    uint8_t rotate_left(uint8_t value, int positions) {
        positions = positions % 8;
        return (value << positions) | (value >> (8 - positions));
    }
    
    /**
     * Multiplica en el campo de Galois GF(2^8)
     * Usado en MixColumns
     */
    uint8_t galois_multiply(uint8_t a, uint8_t b) {
        uint8_t result = 0;
        uint8_t high_bit;
        
        for (int i = 0; i < 8; i++) {
            if (b & 1) {
                result ^= a;
            }
            high_bit = a & 0x80;
            a <<= 1;
            if (high_bit) {
                a ^= 0x1B; // Polinomio irreducible x^8 + x^4 + x^3 + x + 1
            }
            b >>= 1;
        }
        
        return result;
    }
    
    // ========================================================================
    // EXPANSIÓN DE CLAVE
    // ========================================================================
    
    /**
     * Expande la clave del usuario a múltiples claves de ronda
     * Genera (NUM_ROUNDS + 1) claves de 16 bytes cada una
     */
    void expand_key(const std::vector<uint8_t>& user_key) {
        key_schedule.clear();
        
        // Copiar la clave original como primera clave de ronda
        key_schedule.insert(key_schedule.end(), user_key.begin(), user_key.end());
        
        // Generar claves de ronda adicionales
        for (int round = 1; round <= NUM_ROUNDS; round++) {
            std::vector<uint8_t> round_key(16);
            
            // Copiar la clave anterior
            for (int i = 0; i < 16; i++) {
                round_key[i] = key_schedule[(round - 1) * 16 + i];
            }
            
            // Aplicar transformaciones (simplificadas)
            // Rotar y sustituir usando S-box
            for (int i = 0; i < 16; i++) {
                round_key[i] = SBOX[rotate_left(round_key[i], round)];
                round_key[i] ^= round; // Agregar número de ronda
            }
            
            key_schedule.insert(key_schedule.end(), round_key.begin(), round_key.end());
        }
        
        std::cout << "  → Clave expandida: " << (NUM_ROUNDS + 1) << " claves de ronda generadas\n";
    }
    
    // ========================================================================
    // OPERACIÓN 1: SubBytes (Sustitución)
    // ========================================================================
    
    /**
     * Aplica la S-box a cada byte del estado
     * Proporciona confusión (no linealidad)
     */
    void sub_bytes(uint8_t state[16]) {
        for (int i = 0; i < 16; i++) {
            state[i] = SBOX[state[i]];
        }
    }
    
    /**
     * SubBytes inverso (usa S-box inversa)
     */
    void inv_sub_bytes(uint8_t state[16]) {
        for (int i = 0; i < 16; i++) {
            state[i] = INV_SBOX[state[i]];
        }
    }
    
    // ========================================================================
    // OPERACIÓN 2: ShiftRows (Permutación de filas)
    // ========================================================================
    
    /**
     * Organiza los 16 bytes como matriz 4x4 y rota cada fila:
     * 
     * [ 0  1  2  3]      [ 0  1  2  3]
     * [ 4  5  6  7]  =>  [ 5  6  7  4]
     * [ 8  9 10 11]      [10 11  8  9]
     * [12 13 14 15]      [15 12 13 14]
     * 
     * Proporciona difusión
     */
    void shift_rows(uint8_t state[16]) {
        uint8_t temp;
        
        // Fila 1: rotar 1 posición a la izquierda
        temp = state[4];
        state[4] = state[5];
        state[5] = state[6];
        state[6] = state[7];
        state[7] = temp;
        
        // Fila 2: rotar 2 posiciones a la izquierda
        temp = state[8];
        state[8] = state[10];
        state[10] = temp;
        temp = state[9];
        state[9] = state[11];
        state[11] = temp;
        
        // Fila 3: rotar 3 posiciones a la izquierda (= 1 a la derecha)
        temp = state[15];
        state[15] = state[14];
        state[14] = state[13];
        state[13] = state[12];
        state[12] = temp;
    }
    
    /**
     * ShiftRows inverso (rotar en dirección opuesta)
     */
    void inv_shift_rows(uint8_t state[16]) {
        uint8_t temp;
        
        // Fila 1: rotar 1 posición a la derecha
        temp = state[7];
        state[7] = state[6];
        state[6] = state[5];
        state[5] = state[4];
        state[4] = temp;
        
        // Fila 2: rotar 2 posiciones a la derecha
        temp = state[8];
        state[8] = state[10];
        state[10] = temp;
        temp = state[9];
        state[9] = state[11];
        state[11] = temp;
        
        // Fila 3: rotar 3 posiciones a la derecha (= 1 a la izquierda)
        temp = state[12];
        state[12] = state[13];
        state[13] = state[14];
        state[14] = state[15];
        state[15] = temp;
    }
    
    // ========================================================================
    // OPERACIÓN 3: MixColumns (Mezcla de columnas - SIMPLIFICADO)
    // ========================================================================
    
    /**
     * Mezcla los bytes dentro de cada columna de la matriz 4x4
     * Versión simplificada: combina bytes con XOR y multiplicación en GF(2^8)
     * 
     * En AES completo, esto usa una matriz de multiplicación específica.
     * Aquí usamos una versión educativa simplificada.
     */
    void mix_columns(uint8_t state[16]) {
        for (int col = 0; col < 4; col++) {
            int base = col * 4;
            uint8_t a = state[base];
            uint8_t b = state[base + 1];
            uint8_t c = state[base + 2];
            uint8_t d = state[base + 3];
            
            // Aplicar transformación lineal simplificada
            state[base]     = galois_multiply(a, 2) ^ galois_multiply(b, 3) ^ c ^ d;
            state[base + 1] = a ^ galois_multiply(b, 2) ^ galois_multiply(c, 3) ^ d;
            state[base + 2] = a ^ b ^ galois_multiply(c, 2) ^ galois_multiply(d, 3);
            state[base + 3] = galois_multiply(a, 3) ^ b ^ c ^ galois_multiply(d, 2);
        }
    }
    
    /**
     * MixColumns inverso (usa matriz inversa)
     */
    void inv_mix_columns(uint8_t state[16]) {
        for (int col = 0; col < 4; col++) {
            int base = col * 4;
            uint8_t a = state[base];
            uint8_t b = state[base + 1];
            uint8_t c = state[base + 2];
            uint8_t d = state[base + 3];
            
            // Aplicar transformación inversa
            state[base]     = galois_multiply(a, 14) ^ galois_multiply(b, 11) ^ 
                              galois_multiply(c, 13) ^ galois_multiply(d, 9);
            state[base + 1] = galois_multiply(a, 9)  ^ galois_multiply(b, 14) ^ 
                              galois_multiply(c, 11) ^ galois_multiply(d, 13);
            state[base + 2] = galois_multiply(a, 13) ^ galois_multiply(b, 9)  ^ 
                              galois_multiply(c, 14) ^ galois_multiply(d, 11);
            state[base + 3] = galois_multiply(a, 11) ^ galois_multiply(b, 13) ^ 
                              galois_multiply(c, 9)  ^ galois_multiply(d, 14);
        }
    }
    
    // ========================================================================
    // OPERACIÓN 4: AddRoundKey (XOR con clave de ronda)
    // ========================================================================
    
    /**
     * Aplica XOR entre el estado y la clave de ronda
     * Es su propia inversa (XOR es reversible)
     */
    void add_round_key(uint8_t state[16], int round) {
        int offset = round * 16;
        for (int i = 0; i < 16; i++) {
            state[i] ^= key_schedule[offset + i];
        }
    }
    
    // ========================================================================
    // ENCRIPTACIÓN Y DESENCRIPTACIÓN DE UN BLOQUE
    // ========================================================================
    
    /**
     * Encripta un bloque de 16 bytes
     */
    void encrypt_block(uint8_t block[16]) {
        // Ronda inicial: solo AddRoundKey
        add_round_key(block, 0);
        
        // Rondas principales
        for (int round = 1; round < NUM_ROUNDS; round++) {
            sub_bytes(block);
            shift_rows(block);
            mix_columns(block);
            add_round_key(block, round);
        }
        
        // Ronda final (sin MixColumns)
        sub_bytes(block);
        shift_rows(block);
        add_round_key(block, NUM_ROUNDS);
    }
    
    /**
     * Desencripta un bloque de 16 bytes
     * Aplica las operaciones en orden inverso
     */
    void decrypt_block(uint8_t block[16]) {
        // Ronda final inversa
        add_round_key(block, NUM_ROUNDS);
        inv_shift_rows(block);
        inv_sub_bytes(block);
        
        // Rondas principales inversas
        for (int round = NUM_ROUNDS - 1; round >= 1; round--) {
            add_round_key(block, round);
            inv_mix_columns(block);
            inv_shift_rows(block);
            inv_sub_bytes(block);
        }
        
        // Ronda inicial inversa
        add_round_key(block, 0);
    }
    
    // ========================================================================
    // PADDING (Relleno para completar bloques de 16 bytes)
    // ========================================================================
    
    /**
     * Aplica PKCS#7 padding
     * Si faltan N bytes para completar un bloque, agrega N bytes con valor N
     */
    std::vector<uint8_t> add_padding(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> padded = data;
        int padding_length = BLOCK_SIZE - (data.size() % BLOCK_SIZE);
        
        // Siempre agregar padding (incluso si es múltiplo de 16)
        for (int i = 0; i < padding_length; i++) {
            padded.push_back(padding_length);
        }
        
        return padded;
    }
    
    /**
     * Remueve el padding PKCS#7
     */
    std::vector<uint8_t> remove_padding(const std::vector<uint8_t>& data) {
        if (data.empty()) return data;
        
        uint8_t padding_length = data.back();
        
        // Validar padding
        if (padding_length > BLOCK_SIZE || padding_length == 0) {
            std::cerr << "  [Advertencia] Padding inválido\n";
            return data;
        }
        
        // Verificar que todos los bytes de padding son correctos
        for (int i = 1; i <= padding_length; i++) {
            if (data[data.size() - i] != padding_length) {
                std::cerr << "  [Advertencia] Padding corrupto\n";
                return data;
            }
        }
        
        // Remover padding
        std::vector<uint8_t> unpadded(data.begin(), data.end() - padding_length);
        return unpadded;
    }
    
public:
    /**
     * Constructor: inicializa con una clave de usuario
     * La clave se expande a 16 bytes si es más corta
     */
    AESCipher(const std::string& user_key) {
        std::cout << "  → Inicializando AES simplificado\n";
        
        // Convertir clave de usuario a 16 bytes
        std::vector<uint8_t> key(KEY_SIZE, 0);
        
        for (size_t i = 0; i < user_key.length() && i < KEY_SIZE; i++) {
            key[i] = user_key[i];
        }
        
        // Si la clave es más corta, usar hash simple para expandir
        if (user_key.length() < KEY_SIZE) {
            uint32_t hash = 5381;
            for (char c : user_key) {
                hash = ((hash << 5) + hash) + c;
            }
            for (size_t i = user_key.length(); i < KEY_SIZE; i++) {
                key[i] = (hash >> (i * 8)) & 0xFF;
            }
        }
        
        std::cout << "  → Clave preparada (" << KEY_SIZE << " bytes)\n";
        
        // Expandir clave
        expand_key(key);
    }
    
    /**
     * ENCRIPTAR: Procesa datos de cualquier longitud
     */
    std::vector<unsigned char> encrypt(const std::vector<unsigned char>& plaintext) {
        if (plaintext.empty()) {
            std::cerr << "  [Error] Datos vacíos para encriptar\n";
            return std::vector<unsigned char>();
        }
        
        std::cout << "  → Encriptando " << plaintext.size() << " bytes con AES\n";
        
        // Agregar padding
        std::vector<uint8_t> padded = add_padding(plaintext);
        std::cout << "  → Tamaño con padding: " << padded.size() << " bytes\n";
        
        // Encriptar bloque por bloque
        std::vector<unsigned char> ciphertext;
        ciphertext.reserve(padded.size());
        
        for (size_t i = 0; i < padded.size(); i += BLOCK_SIZE) {
            uint8_t block[BLOCK_SIZE];
            
            // Copiar bloque
            for (int j = 0; j < BLOCK_SIZE; j++) {
                block[j] = padded[i + j];
            }
            
            // Encriptar bloque
            encrypt_block(block);
            
            // Agregar al resultado
            for (int j = 0; j < BLOCK_SIZE; j++) {
                ciphertext.push_back(block[j]);
            }
        }
        
        std::cout << "  → Encriptación completada\n";
        
        return ciphertext;
    }
    
    /**
     * DESENCRIPTAR: Recupera los datos originales
     */
    std::vector<unsigned char> decrypt(const std::vector<unsigned char>& ciphertext) {
        if (ciphertext.empty()) {
            std::cerr << "  [Error] Datos vacíos para desencriptar\n";
            return std::vector<unsigned char>();
        }
        
        if (ciphertext.size() % BLOCK_SIZE != 0) {
            std::cerr << "  [Error] Tamaño de datos inválido (debe ser múltiplo de " 
                      << BLOCK_SIZE << ")\n";
            return std::vector<unsigned char>();
        }
        
        std::cout << "  → Desencriptando " << ciphertext.size() << " bytes con AES\n";
        
        // Desencriptar bloque por bloque
        std::vector<uint8_t> decrypted;
        decrypted.reserve(ciphertext.size());
        
        for (size_t i = 0; i < ciphertext.size(); i += BLOCK_SIZE) {
            uint8_t block[BLOCK_SIZE];
            
            // Copiar bloque
            for (int j = 0; j < BLOCK_SIZE; j++) {
                block[j] = ciphertext[i + j];
            }
            
            // Desencriptar bloque
            decrypt_block(block);
            
            // Agregar al resultado
            for (int j = 0; j < BLOCK_SIZE; j++) {
                decrypted.push_back(block[j]);
            }
        }
        
        // Remover padding
        std::vector<unsigned char> plaintext = remove_padding(decrypted);
        std::cout << "  → Desencriptación completada (" << plaintext.size() << " bytes)\n";
        
        return plaintext;
    }
};

// ============================================================================
// TABLA S-BOX DE AES (estándar)
// ============================================================================

const uint8_t AESCipher::SBOX[256] = {
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

const uint8_t AESCipher::INV_SBOX[256] = {
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

#endif // AES_CIPHER_H