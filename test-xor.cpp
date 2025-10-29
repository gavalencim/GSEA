#include "xor.h"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== Prueba de Encriptación XOR Mejorada ===\n\n";
    
    // Texto de prueba
    std::string secret_message = "Este es un mensaje super secreto!";
    std::cout << "Mensaje original: \"" << secret_message << "\"\n";
    std::cout << "Tamaño: " << secret_message.length() << " bytes\n\n";
    
    // Convertir a vector de bytes
    std::vector<unsigned char> plaintext(secret_message.begin(), secret_message.end());
    
    // Clave secreta
    std::string password = "MiClaveSecreta123";
    std::cout << "Usando clave: \"" << password << "\"\n\n";
    
    // Crear el cifrador con la clave
    XORCipher cipher(password);
    
    // ENCRIPTAR
    std::cout << "--- ENCRIPTANDO ---\n";
    std::vector<unsigned char> encrypted = cipher.encrypt(plaintext);
    
    if (encrypted.empty()) {
        std::cerr << "Error en la encriptación\n";
        return 1;
    }
    
    std::cout << "Datos encriptados (hex): ";
    XORCipher::print_hex(encrypted, 32);
    std::cout << "\n";
    
    // DESENCRIPTAR
    std::cout << "--- DESENCRIPTANDO ---\n";
    std::vector<unsigned char> decrypted = cipher.decrypt(encrypted);
    
    if (decrypted.empty()) {
        std::cerr << "Error en la desencriptación\n";
        return 1;
    }
    
    // Convertir resultado a string
    std::string recovered_message(decrypted.begin(), decrypted.end());
    
    // VERIFICAR
    std::cout << "\n--- VERIFICACIÓN ---\n";
    std::cout << "Mensaje recuperado: \"" << recovered_message << "\"\n";
    std::cout << "Tamaño: " << recovered_message.length() << " bytes\n\n";
    
    if (secret_message == recovered_message) {
        std::cout << "✓ ¡ÉXITO! El mensaje fue encriptado y desencriptado correctamente\n";
    } else {
        std::cout << "✗ ERROR: Los mensajes no coinciden\n";
        return 1;
    }
    
    // Prueba con clave incorrecta
    std::cout << "\n--- PRUEBA CON CLAVE INCORRECTA ---\n";
    std::string wrong_password = "ClaveIncorrecta";
    std::cout << "Intentando desencriptar con: \"" << wrong_password << "\"\n";
    
    XORCipher wrong_cipher(wrong_password);
    std::vector<unsigned char> wrong_decrypted = wrong_cipher.decrypt(encrypted);
    std::string wrong_message(wrong_decrypted.begin(), wrong_decrypted.end());
    
    std::cout << "Resultado: \"" << wrong_message << "\"\n";
    std::cout << "→ Como puedes ver, con la clave incorrecta obtienes basura\n";
    
    std::cout << "\n=== Prueba completada exitosamente ===\n";
    
    return 0;
}