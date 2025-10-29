#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>
#include <cstring>
#include <iostream>

// Incluir las librerías necesarias según el sistema operativo
#ifdef _WIN32
    // Windows
    #include <windows.h>
    #include <fileapi.h>
#else
    // Linux/POSIX
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <errno.h>
#endif

// Clase para manejar la lectura y escritura de archivos
class FileManager {
private:
    std::string filepath;
    
#ifdef _WIN32
    HANDLE file_handle;  // Descriptor de archivo en Windows, puntero especial al archivo
#else
    int file_descriptor;  // Descriptor de archivo en Linux
#endif

public:
    // Constructor
    FileManager(const std::string& path) : filepath(path) {
#ifdef _WIN32
        file_handle = INVALID_HANDLE_VALUE;
#else
        file_descriptor = -1;
#endif
    }
    
    // Destructor - cierra el archivo si está abierto
    ~FileManager() {
        close_file();
    }
    
    // Abrir archivo para lectura
    bool open_for_reading() {
#ifdef _WIN32
        // Windows: CreateFile para abrir
        file_handle = CreateFileA(
            filepath.c_str(),           // Nombre del archivo
            GENERIC_READ,               // Acceso de lectura
            FILE_SHARE_READ,            // Permitir compartir lectura
            nullptr,                    // Seguridad por defecto
            OPEN_EXISTING,              // Solo abrir si existe
            FILE_ATTRIBUTE_NORMAL,      // Atributos normales
            nullptr                     // Sin plantilla
        );
        
        if (file_handle == INVALID_HANDLE_VALUE) {
            std::cerr << "Error al abrir archivo para lectura: " << filepath << "\n";
            std::cerr << "Código de error: " << GetLastError() << "\n";
            return false;
        }
#else
        // Linux: open() con flags de solo lectura
        file_descriptor = open(filepath.c_str(), O_RDONLY);
        
        if (file_descriptor == -1) {
            std::cerr << "Error al abrir archivo para lectura: " << filepath << "\n";
            std::cerr << "Error: " << strerror(errno) << "\n";
            return false;
        }
#endif
        return true;
    }
    
    // Abrir archivo para escritura (crea el archivo si no existe)
    bool open_for_writing() {
#ifdef _WIN32
        // Windows: CreateFile para escribir
        file_handle = CreateFileA(
            filepath.c_str(),           // Nombre del archivo
            GENERIC_WRITE,              // Acceso de escritura
            0,                          // No compartir
            nullptr,                    // Seguridad por defecto
            CREATE_ALWAYS,              // Crear siempre (sobrescribe si existe)
            FILE_ATTRIBUTE_NORMAL,      // Atributos normales
            nullptr                     // Sin plantilla
        );
        
        if (file_handle == INVALID_HANDLE_VALUE) {
            std::cerr << "Error al abrir archivo para escritura: " << filepath << "\n";
            std::cerr << "Código de error: " << GetLastError() << "\n";
            return false;
        }
#else
        // Linux: open() con flags de escritura
        // O_WRONLY = solo escritura
        // O_CREAT = crear si no existe
        // O_TRUNC = truncar (vaciar) si existe
        // 0644 = permisos: rw-r--r-- (lectura/escritura para dueño, solo lectura para otros)
        file_descriptor = open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        
        if (file_descriptor == -1) {
            std::cerr << "Error al abrir archivo para escritura: " << filepath << "\n";
            std::cerr << "Error: " << strerror(errno) << "\n";
            return false;
        }
#endif
        return true;
    }
    
    // Leer datos del archivo
    // Retorna un vector con los bytes leídos
    std::vector<unsigned char> read_file() {
        std::vector<unsigned char> data; // Vector de cualquier tamano (0-255 bytes)
        
        if (!open_for_reading()) {
            return data; // Retorna vector vacío si falla
        }
        
        // Obtener el tamaño del archivo
        long file_size = get_file_size();
        if (file_size <= 0) {
            close_file();
            return data;
        }
        
        // Reservar espacio en el vector
        data.resize(file_size);
        
#ifdef _WIN32
        // Windows: ReadFile
        DWORD bytes_read = 0;
        BOOL result = ReadFile(
            file_handle,                // Handle del archivo
            data.data(),                // Buffer donde guardar los datos
            file_size,                  // Cantidad de bytes a leer
            &bytes_read,                // Bytes realmente leídos
            nullptr                     // Sin operación asíncrona
        );
        
        if (!result || bytes_read != file_size) {
            std::cerr << "Error al leer el archivo\n";
            data.clear();
        }
#else
        // Linux: read()
        ssize_t bytes_read = read(file_descriptor, data.data(), file_size);
        
        if (bytes_read != file_size) {
            std::cerr << "Error al leer el archivo: " << strerror(errno) << "\n";
            data.clear();
        }
#endif
        
        close_file();
        return data;
    }
    
    // Escribir datos al archivo
    bool write_file(const std::vector<unsigned char>& data) {
        if (!open_for_writing()) {
            return false;
        }
        
        if (data.empty()) {
            close_file();
            return true; // Archivo vacío es válido
        }
        
#ifdef _WIN32
        // Windows: WriteFile
        DWORD bytes_written = 0;
        BOOL result = WriteFile(
            file_handle,                // Handle del archivo
            data.data(),                // Buffer con los datos a escribir
            data.size(),                // Cantidad de bytes a escribir
            &bytes_written,             // Bytes realmente escritos
            nullptr                     // Sin operación asíncrona
        );
        
        if (!result || bytes_written != data.size()) {
            std::cerr << "Error al escribir el archivo\n";
            close_file();
            return false;
        }
#else
        // Linux: write()
        ssize_t bytes_written = write(file_descriptor, data.data(), data.size());
        
        if (bytes_written != (ssize_t)data.size()) {
            std::cerr << "Error al escribir el archivo: " << strerror(errno) << "\n";
            close_file();
            return false;
        }
#endif
        
        close_file();
        return true;
    }
    
    // Obtener el tamaño del archivo
    long get_file_size() {
#ifdef _WIN32
        // Windows: GetFileSize
        DWORD file_size = GetFileSize(file_handle, nullptr);
        if (file_size == INVALID_FILE_SIZE) {
            std::cerr << "Error al obtener tamaño del archivo\n";
            return -1;
        }
        return (long)file_size;
#else
        // Linux: fstat
        struct stat file_stat;
        if (fstat(file_descriptor, &file_stat) == -1) {
            std::cerr << "Error al obtener tamaño del archivo: " << strerror(errno) << "\n";
            return -1;
        }
        return (long)file_stat.st_size;
#endif
    }
    
    // Cerrar el archivo
    void close_file() {
#ifdef _WIN32
        if (file_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(file_handle);
            file_handle = INVALID_HANDLE_VALUE;
        }
#else
        if (file_descriptor != -1) {
            close(file_descriptor);
            file_descriptor = -1;
        }
#endif
    }
    
    // Verificar si un archivo existe
    static bool file_exists(const std::string& path) {
#ifdef _WIN32
        DWORD attributes = GetFileAttributesA(path.c_str());
        return (attributes != INVALID_FILE_ATTRIBUTES && 
                !(attributes & FILE_ATTRIBUTE_DIRECTORY));
#else
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
#endif
    }
    
    // Verificar si es un directorio
    static bool is_directory(const std::string& path) {
#ifdef _WIN32
        DWORD attributes = GetFileAttributesA(path.c_str());
        return (attributes != INVALID_FILE_ATTRIBUTES && 
                (attributes & FILE_ATTRIBUTE_DIRECTORY));
#else
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
#endif
    }
    
    // Listar archivos en un directorio
    static std::vector<std::string> list_files(const std::string& dir_path) {
        std::vector<std::string> files;
        
#ifdef _WIN32
        // Windows: FindFirstFile / FindNextFile
        WIN32_FIND_DATAA find_data;
        std::string search_path = dir_path + "\\*";
        HANDLE find_handle = FindFirstFileA(search_path.c_str(), &find_data);
        
        if (find_handle == INVALID_HANDLE_VALUE) {
            std::cerr << "Error al listar directorio: " << dir_path << "\n";
            return files;
        }
        
        do {
            std::string filename = find_data.cFileName;
            
            // Ignorar "." y ".."
            if (filename == "." || filename == "..") {
                continue;
            }
            
            // Si no es un directorio, agregarlo a la lista
            if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(dir_path + "\\" + filename);
            }
        } while (FindNextFileA(find_handle, &find_data));
        
        FindClose(find_handle);
#else
        // Linux: opendir / readdir
        DIR* dir = opendir(dir_path.c_str());
        
        if (dir == nullptr) {
            std::cerr << "Error al abrir directorio: " << dir_path << "\n";
            std::cerr << "Error: " << strerror(errno) << "\n";
            return files;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string filename = entry->d_name;
            
            // Ignorar "." y ".."
            if (filename == "." || filename == "..") {
                continue;
            }
            
            std::string full_path = dir_path + "/" + filename;
            
            // Verificar que sea un archivo regular
            if (file_exists(full_path)) {
                files.push_back(full_path);
            }
        }
        
        closedir(dir);
#endif
        
        return files;
    }
};

#endif // FILE_MANAGER_H