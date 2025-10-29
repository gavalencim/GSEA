#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include "huffman.h"
#include "aes-chiper.h"

// Librerías para syscalls de Linux
#include <unistd.h>      // open, read, write, close
#include <fcntl.h>       // Flags para open (O_RDONLY, O_WRONLY, etc.)
#include <sys/stat.h>    // fstat, stat
#include <sys/types.h>   // Tipos de datos para syscalls
#include <dirent.h>      // opendir, readdir, closedir
#include <errno.h>       // errno para errores

// Estructura para almacenar la configuración del programa
struct Config {
    // Operaciones a realizar
    bool compress = false;      // -c: comprimir
    bool decompress = false;    // -d: descomprimir
    bool encrypt = false;       // -e: encriptar
    bool decrypt = false;       // -u: desencriptar (u = unlock)
    
    // Rutas de entrada y salida
    std::string input_path;     // -i: ruta del archivo o directorio
    std::string output_path;    // -o: ruta de salida
    
    // Algoritmos
    std::string comp_algorithm = "huffman";  // --comp-alg
    std::string enc_algorithm = "xor";       // --enc-alg
    
    // Clave de encriptación
    std::string key;            // -k: clave secreta
    
    // Flag para verificar si la configuración es válida
    bool is_valid = true;
};

// ============================================================================
// FUNCIONES DE LECTURA/ESCRITURA DE ARCHIVOS CON SYSCALLS DE LINUX
// ============================================================================

/**
 * Función para LEER un archivo completo usando syscalls de Linux
 * 
 * Syscalls usadas:
 *   - open()  : Abre el archivo y retorna un file descriptor (fd)
 *   - fstat() : Obtiene información del archivo (incluyendo tamaño)
 *   - read()  : Lee bytes del archivo
 *   - close() : Cierra el file descriptor
 * 
 * @param filepath Ruta del archivo a leer
 * @return Vector con los bytes del archivo (vacío si hay error)
 */
std::vector<unsigned char> read_file_syscall(const std::string& filepath) {
    std::vector<unsigned char> data;
    
    std::cout << "  [Syscall] Abriendo archivo para lectura: " << filepath << "\n";
    
    // PASO 1: Abrir el archivo con open()
    // O_RDONLY = Open for Reading ONLY (solo lectura)
    int fd = open(filepath.c_str(), O_RDONLY);
    
    // Verificar si open() falló
    if (fd == -1) {
        // fd == -1 significa error
        // errno contiene el código de error
        // strerror(errno) convierte el código a texto legible
        std::cerr << "  [Error] open() falló: " << strerror(errno) << "\n";
        return data;  // Retornar vector vacío
    }
    
    std::cout << "  [Syscall] ✓ open() exitoso - File descriptor (fd) = " << fd << "\n";
    
    // PASO 2: Obtener información del archivo con fstat()
    // struct stat es una estructura que contiene metadata del archivo
    struct stat file_stat;
    
    if (fstat(fd, &file_stat) == -1) {
        // fstat() retorna -1 si hay error
        std::cerr << "  [Error] fstat() falló: " << strerror(errno) << "\n";
        close(fd);  // Cerrar el fd antes de salir
        return data;
    }
    
    // file_stat.st_size contiene el tamaño del archivo en bytes
    long file_size = file_stat.st_size;
    std::cout << "  [Syscall] ✓ fstat() exitoso - Tamaño: " << file_size << " bytes\n";
    
    // PASO 3: Reservar espacio en memoria para los datos
    // resize() ajusta el tamaño del vector al tamaño del archivo
    data.resize(file_size);
    std::cout << "  [Memoria] Vector redimensionado a " << file_size << " bytes\n";
    
    // PASO 4: Leer el archivo con read()
    std::cout << "  [Syscall] Llamando a read()...\n";
    
    // read(fd, buffer, count):
    //   - fd: file descriptor del archivo abierto
    //   - buffer: puntero a donde guardar los datos (data.data())
    //   - count: cantidad de bytes a leer
    // Retorna: cantidad de bytes leídos (o -1 si error)
    ssize_t bytes_read = read(fd, data.data(), file_size);
    
    // Verificar si read() fue exitoso
    if (bytes_read == -1) {
        std::cerr << "  [Error] read() falló: " << strerror(errno) << "\n";
        data.clear();
    } else if (bytes_read != file_size) {
        // Se leyeron menos bytes de los esperados
        std::cerr << "  [Advertencia] read() incompleto\n";
        std::cerr << "  [Advertencia] Esperados: " << file_size << " bytes\n";
        std::cerr << "  [Advertencia] Leídos: " << bytes_read << " bytes\n";
        data.resize(bytes_read);  // Ajustar al tamaño real
    } else {
        std::cout << "  [Syscall] ✓ read() exitoso - " << bytes_read << " bytes leídos\n";
    }
    
    // PASO 5: Cerrar el archivo con close()
    // Siempre debemos cerrar los file descriptors que abrimos
    close(fd);
    std::cout << "  [Syscall] ✓ close() - File descriptor cerrado\n";
    
    return data;
}

/**
 * Función para ESCRIBIR un archivo completo usando syscalls de Linux
 * 
 * Syscalls usadas:
 *   - open()  : Crea/abre el archivo
 *   - write() : Escribe bytes al archivo
 *   - close() : Cierra el file descriptor
 * 
 * @param filepath Ruta del archivo a escribir
 * @param data Vector con los bytes a escribir
 * @return true si fue exitoso, false si hubo error
 */
bool write_file_syscall(const std::string& filepath, const std::vector<unsigned char>& data) {
    std::cout << "  [Syscall] Abriendo archivo para escritura: " << filepath << "\n";
    
    if (data.empty()) {
        std::cout << "  [Advertencia] Datos vacíos, creando archivo vacío\n";
    }
    
    // PASO 1: Crear/abrir el archivo con open()
    // Flags explicados:
    //   O_WRONLY = Open for Writing ONLY (solo escritura)
    //   O_CREAT  = CREATE file if it doesn't exist (crear si no existe)
    //   O_TRUNC  = TRUNCATE file to 0 bytes if it exists (vaciar si existe)
    // 
    // El tercer parámetro (0644) son los permisos en octal:
    //   0644 = rw-r--r-- 
    //   Owner: read(4) + write(2) = 6
    //   Group: read(4) = 4
    //   Others: read(4) = 4
    int fd = open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    // Verificar si open() falló
    if (fd == -1) {
        std::cerr << "  [Error] open() falló: " << strerror(errno) << "\n";
        return false;
    }
    
    std::cout << "  [Syscall] ✓ open() exitoso - File descriptor (fd) = " << fd << "\n";
    
    // PASO 2: Escribir datos con write() (si hay datos)
    if (!data.empty()) {
        std::cout << "  [Syscall] Llamando a write() para " << data.size() << " bytes...\n";
        
        // write(fd, buffer, count):
        //   - fd: file descriptor del archivo abierto
        //   - buffer: puntero a los datos a escribir
        //   - count: cantidad de bytes a escribir
        // Retorna: cantidad de bytes escritos (o -1 si error)
        ssize_t bytes_written = write(fd, data.data(), data.size());
        
        // Verificar si write() fue exitoso
        if (bytes_written == -1) {
            std::cerr << "  [Error] write() falló: " << strerror(errno) << "\n";
            close(fd);
            return false;
        } else if (bytes_written != (ssize_t)data.size()) {
            // Se escribieron menos bytes de los esperados
            std::cerr << "  [Error] write() incompleto\n";
            std::cerr << "  [Error] Esperados: " << data.size() << " bytes\n";
            std::cerr << "  [Error] Escritos: " << bytes_written << " bytes\n";
            close(fd);
            return false;
        } else {
            std::cout << "  [Syscall] ✓ write() exitoso - " << bytes_written << " bytes escritos\n";
        }
    }
    
    // PASO 3: Cerrar el archivo con close()
    close(fd);
    std::cout << "  [Syscall] ✓ close() - File descriptor cerrado\n";
    
    return true;
}

/**
 * Función para verificar si un archivo existe
 * Usa la syscall stat()
 */
bool file_exists(const std::string& path) {
    struct stat buffer;
    // stat() retorna 0 si el archivo existe
    // S_ISREG() verifica que sea un archivo regular (no directorio, no link)
    return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

/**
 * Función para verificar si es un directorio
 * Usa la syscall stat()
 */
bool is_directory(const std::string& path) {
    struct stat buffer;
    // S_ISDIR() verifica que sea un directorio
    return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

/**
 * Función para listar todos los archivos en un directorio
 * 
 * Syscalls usadas:
 *   - opendir()  : Abre el directorio
 *   - readdir()  : Lee una entrada del directorio
 *   - closedir() : Cierra el directorio
 */
std::vector<std::string> list_files(const std::string& dir_path) {
    std::vector<std::string> files;
    
    std::cout << "  [Syscall] Abriendo directorio: " << dir_path << "\n";
    
    // PASO 1: Abrir el directorio con opendir()
    DIR* dir = opendir(dir_path.c_str());
    
    if (dir == nullptr) {
        // opendir() retorna nullptr si hay error
        std::cerr << "  [Error] opendir() falló: " << strerror(errno) << "\n";
        return files;
    }
    
    std::cout << "  [Syscall] ✓ opendir() exitoso\n";
    
    // PASO 2: Leer entradas del directorio con readdir()
    // struct dirent contiene información de cada entrada (archivo/carpeta)
    struct dirent* entry;
    
    // readdir() retorna nullptr cuando no hay más entradas
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        
        // Ignorar "." (directorio actual) y ".." (directorio padre)
        if (filename == "." || filename == "..") {
            continue;
        }
        
        // Construir la ruta completa
        std::string full_path = dir_path + "/" + filename;
        
        // Verificar que sea un archivo regular (no un subdirectorio)
        if (file_exists(full_path)) {
            files.push_back(full_path);
            std::cout << "    - Encontrado: " << filename << "\n";
        }
    }
    
    // PASO 3: Cerrar el directorio con closedir()
    closedir(dir);
    std::cout << "  [Syscall] ✓ closedir() - Directorio cerrado\n";
    std::cout << "  [Syscall] Total archivos: " << files.size() << "\n";
    
    return files;
}

// ============================================================================
// FUNCIONES DE PARSEO DE ARGUMENTOS Y CONFIGURACIÓN
// ============================================================================

void print_usage(const char* program_name) {
    std::cout << "Uso: " << program_name << " [opciones]\n\n";
    std::cout << "Opciones obligatorias:\n";
    std::cout << "  -i <ruta>        Archivo o directorio de entrada\n";
    std::cout << "  -o <ruta>        Archivo o directorio de salida\n\n";
    std::cout << "Operaciones (al menos una):\n";
    std::cout << "  -c               Comprimir\n";
    std::cout << "  -d               Descomprimir\n";
    std::cout << "  -e               Encriptar\n";
    std::cout << "  -u               Desencriptar\n";
    std::cout << "  (Pueden combinarse: -ce = comprimir y encriptar)\n\n";
    std::cout << "Opciones adicionales:\n";
    std::cout << "  --comp-alg <alg> Algoritmo de compresión (default: huffman)\n";
    std::cout << "  --enc-alg <alg>  Algoritmo de encriptación (default: AES simplificado)\n";
    std::cout << "  -k <clave>       Clave secreta para encriptación\n\n";
    std::cout << "Ejemplos:\n";
    std::cout << "  " << program_name << " -c -i archivo.txt -o archivo.huff\n";
    std::cout << "  " << program_name << " -ce -i doc.pdf -o doc.gsea -k miClave\n";
    std::cout << "  " << program_name << " -d -i archivo.huff -o archivo.txt\n";
    std::cout << "  " << program_name << " -du -i doc.gsea -o doc.pdf -k miClave\n";
}

Config parse_arguments(int argc, char* argv[]) {
    Config config;
    
    if (argc < 2) {
        print_usage(argv[0]);
        config.is_valid = false;
        return config;
    }
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg[0] == '-' && arg[1] != '-') {
            for (size_t j = 1; j < arg.length(); j++) {
                switch (arg[j]) {
                    case 'c': config.compress = true; break;
                    case 'd': config.decompress = true; break;
                    case 'e': config.encrypt = true; break;
                    case 'u': config.decrypt = true; break;
                    case 'i':
                        if (i + 1 < argc) {
                            config.input_path = argv[++i];
                        } else {
                            std::cerr << "Error: -i requiere un argumento\n";
                            config.is_valid = false;
                        }
                        j = arg.length();
                        break;
                    case 'o':
                        if (i + 1 < argc) {
                            config.output_path = argv[++i];
                        } else {
                            std::cerr << "Error: -o requiere un argumento\n";
                            config.is_valid = false;
                        }
                        j = arg.length();
                        break;
                    case 'k':
                        if (i + 1 < argc) {
                            config.key = argv[++i];
                        } else {
                            std::cerr << "Error: -k requiere un argumento\n";
                            config.is_valid = false;
                        }
                        j = arg.length();
                        break;
                    default:
                        std::cerr << "Error: Opción desconocida -" << arg[j] << "\n";
                        config.is_valid = false;
                }
            }
        }
        else if (arg == "--comp-alg") {
            if (i + 1 < argc) {
                config.comp_algorithm = argv[++i];
            } else {
                std::cerr << "Error: --comp-alg requiere un argumento\n";
                config.is_valid = false;
            }
        }
        else if (arg == "--enc-alg") {
            if (i + 1 < argc) {
                config.enc_algorithm = argv[++i];
            } else {
                std::cerr << "Error: --enc-alg requiere un argumento\n";
                config.is_valid = false;
            }
        }
    }
    
    // Validaciones
    if (config.input_path.empty()) {
        std::cerr << "Error: Debe especificar archivo de entrada con -i\n";
        config.is_valid = false;
    }
    
    if (config.output_path.empty()) {
        std::cerr << "Error: Debe especificar archivo de salida con -o\n";
        config.is_valid = false;
    }
    
    if (!config.compress && !config.decompress && !config.encrypt && !config.decrypt) {
        std::cerr << "Error: Debe especificar al menos una operación (-c, -d, -e, -u)\n";
        config.is_valid = false;
    }
    
    if ((config.encrypt || config.decrypt) && config.key.empty()) {
        std::cerr << "Error: La encriptación/desencriptación requiere una clave (-k)\n";
        config.is_valid = false;
    }
    
    if (config.compress && config.decompress) {
        std::cerr << "Error: No se puede comprimir y descomprimir simultáneamente\n";
        config.is_valid = false;
    }
    
    if (config.encrypt && config.decrypt) {
        std::cerr << "Error: No se puede encriptar y desencriptar simultáneamente\n";
        config.is_valid = false;
    }
    
    return config;
}

void print_config(const Config& config) {
    std::cout << "\n═══════════════════════════════════════════════════════\n";
    std::cout << "  CONFIGURACIÓN\n";
    std::cout << "═══════════════════════════════════════════════════════\n";
    std::cout << "  Entrada:     " << config.input_path << "\n";
    std::cout << "  Salida:      " << config.output_path << "\n";
    std::cout << "  Operaciones: ";
    if (config.compress) std::cout << "comprimir ";
    if (config.decompress) std::cout << "descomprimir ";
    if (config.encrypt) std::cout << "encriptar ";
    if (config.decrypt) std::cout << "desencriptar ";
    std::cout << "\n";
    if (!config.key.empty()) {
        std::cout << "  Clave:       [***oculta***]\n";
    }
    std::cout << "  Compresión:  " << config.comp_algorithm << "\n";
    std::cout << "  Encriptación: " << config.enc_algorithm << "\n";
    std::cout << "═══════════════════════════════════════════════════════\n\n";
}

// ============================================================================
// FUNCIÓN PRINCIPAL DE PROCESAMIENTO
// ============================================================================

bool process_file(const std::string& input_file, const std::string& output_file, const Config& config) {
    std::cout << "\n┌───────────────────────────────────────────────────────┐\n";
    std::cout << "│ PROCESANDO: " << input_file << "\n";
    std::cout << "│ DESTINO:    " << output_file << "\n";
    std::cout << "└───────────────────────────────────────────────────────┘\n";
    
    // PASO 1: Leer el archivo con syscalls
    std::cout << "\n[PASO 1: LECTURA CON SYSCALLS]\n";
    std::vector<unsigned char> data = read_file_syscall(input_file);
    
    if (data.empty()) {
        std::cerr << "\n✗ ERROR CRÍTICO: No se pudo leer el archivo o está vacío\n";
        std::cerr << "  Archivo: " << input_file << "\n";
        return false;
    }
    
    std::cout << "\n✓ Lectura completada exitosamente\n";
    std::cout << "  Bytes leídos: " << data.size() << "\n";
    std::cout << "  Primeros bytes (hex): ";
    for (size_t i = 0; i < std::min(data.size(), (size_t)16); i++) {
        printf("%02x ", data[i]);
    }
    std::cout << "\n";
    
    // PASO 2: Aplicar operaciones según configuración
    
    // Orden para comprimir + encriptar: COMPRIMIR PRIMERO
    if (config.compress) {
        std::cout << "\n[PASO 2: COMPRESIÓN HUFFMAN]\n";
        std::cout << "  Tamaño antes de comprimir: " << data.size() << " bytes\n";
        HuffmanCoder huffman;
        //data = huffman.compress(data);
        std::vector<unsigned char> compressed = huffman.compress(data);
        
        if (compressed.empty()) {
            std::cerr << "\n✗ Error: Fallo en la compresión\n";
            return false;
        }

        std::cout << "  Tamaño después de comprimir: " << compressed.size() << " bytes\n";
        std::cout << "  Ratio: " << (100.0 * compressed.size() / data.size()) << "%\n";
        
        data = compressed;  // Actualizar datos
    }
    
    if (config.encrypt) {
        std::cout << "\n[PASO 3: ENCRIPTACIÓN]\n";
        AESCipher cipher(config.key);
        //data = cipher.encrypt(data);
        std::vector<unsigned char> encrypted = cipher.encrypt(data);
        
        if (encrypted.empty()) {
            std::cerr << "\n✗ Error: Fallo en la encriptación\n";
            return false;
        }

        data = encrypted;  // Actualizar datos
    }
    
    // Orden para desencriptar + descomprimir: DESENCRIPTAR PRIMERO
    if (config.decrypt) {
        std::cout << "\n[PASO 2: DESENCRIPTACIÓN]\n";
        AESCipher cipher(config.key);
        //data = cipher.decrypt(data);
        std::vector<unsigned char> decrypted = cipher.decrypt(data);
        
        if (decrypted.empty()) {
            std::cerr << "\n✗ Error: Fallo en la desencriptación\n";
            return false;
        }

        data = decrypted;  // Actualizar datos
    }
    
    if (config.decompress) {
        std::cout << "\n[PASO 3: DESCOMPRESIÓN]\n";
        std::cout << "  Tamaño antes de descomprimir: " << data.size() << " bytes\n";
        HuffmanCoder huffman;
        std::vector<unsigned char> decompressed = huffman.decompress(data);
        
        if (decompressed.empty()) {
            std::cerr << "\n✗ Error: Fallo en la descompresión\n";
            return false;
        }

        std::cout << "  Tamaño después de descomprimir: " << decompressed.size() << " bytes\n";
        
        data = decompressed;  // Actualizar datos
    }
    
    // PASO 3: Escribir el resultado con syscalls
    std::cout << "\n[PASO 4: ESCRITURA CON SYSCALLS]\n";
    if (!write_file_syscall(output_file, data)) {
        std::cerr << "\n✗ Error: No se pudo escribir el archivo\n";
        return false;
    }
    
    std::cout << "\n✓ Archivo procesado exitosamente\n";
    std::cout << "  → Guardado en: " << output_file << " (" << data.size() << " bytes)\n";
    
    return true;
}

// ============================================================================
// FUNCIÓN MAIN
// ============================================================================

int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                        ║\n";
    std::cout << "║  GSEA - Gestión Segura y Eficiente de Archivos       ║\n";
    std::cout << "║  Universidad EAFIT - Sistemas Operativos             ║\n";
    std::cout << "║                                                        ║\n";
    std::cout << "║  Usando syscalls POSIX directas:                      ║\n";
    std::cout << "║    • open() / read() / write() / close()              ║\n";
    std::cout << "║    • opendir() / readdir() / closedir()               ║\n";
    std::cout << "║    • stat() / fstat()                                 ║\n";
    std::cout << "║                                                        ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    
    // Parsear argumentos
    Config config = parse_arguments(argc, argv);
    
    if (!config.is_valid) {
        return 1;
    }
    
    // Mostrar configuración
    print_config(config);
    
    // Verificar que la entrada existe
    bool input_is_directory = is_directory(config.input_path);
    bool input_exists = file_exists(config.input_path) || input_is_directory;
    
    if (!input_exists) {
        std::cerr << "✗ Error: La ruta de entrada no existe: " << config.input_path << "\n";
        return 1;
    }
    
    bool success = false;
    
    if (input_is_directory) {
        // CASO 1: Procesar directorio completo
        std::cout << "→ Tipo de entrada: DIRECTORIO\n\n";
        std::vector<std::string> files = list_files(config.input_path);
        
        if (files.empty()) {
            std::cerr << "✗ Error: No hay archivos en el directorio\n";
            return 1;
        }
        
        std::cout << "\n→ Total de archivos a procesar: " << files.size() << "\n";
        
        int processed = 0;
        int failed = 0;
        
        for (const std::string& input_file : files) {
            // Extraer nombre del archivo
            size_t last_slash = input_file.find_last_of('/');
            std::string filename = (last_slash != std::string::npos) 
                                   ? input_file.substr(last_slash + 1) 
                                   : input_file;
            
            // Construir ruta de salida
            std::string output_file = config.output_path + "/" + filename;
            
            // Agregar extensión apropiada
            if (config.compress && config.encrypt) {
                output_file += ".gsea";
            } else if (config.compress) {
                output_file += ".huff";
            } else if (config.encrypt) {
                output_file += ".enc";
            }
            
            // Procesar archivo
            if (process_file(input_file, output_file, config)) {
                processed++;
            } else {
                failed++;
            }
        }
        
        // Resumen
        std::cout << "\n╔════════════════════════════════════════════════════════╗\n";
        std::cout << "║  RESUMEN                                               ║\n";
        std::cout << "╚════════════════════════════════════════════════════════╝\n";
        std::cout << "  Archivos procesados: " << processed << "\n";
        std::cout << "  Archivos fallidos:   " << failed << "\n\n";
        
        success = (processed > 0);
        
    } else {
        // CASO 2: Procesar archivo individual
        std::cout << "→ Tipo de entrada: ARCHIVO INDIVIDUAL\n";
        success = process_file(config.input_path, config.output_path, config);
    }
    
    // Mensaje final
    if (success) {
        std::cout << "╔════════════════════════════════════════════════════════╗\n";
        std::cout << "║  ✓ PROCESO COMPLETADO EXITOSAMENTE                    ║\n";
        std::cout << "╚════════════════════════════════════════════════════════╝\n\n";
        return 0;
    } else {
        std::cout << "╔════════════════════════════════════════════════════════╗\n";
        std::cout << "║  ✗ PROCESO TERMINADO CON ERRORES                      ║\n";
        std::cout << "╚════════════════════════════════════════════════════════╝\n\n";
        return 1;
    }
}