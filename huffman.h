#ifndef HUFFMAN_H
#define HUFFMAN_H

//POSIX (obligado para los hilos) buscar, hacer funciones, caché y buffer 4k (4096)

#include <vector>
#include <map>
#include <queue>
#include <string>
#include <iostream>
#include <cstring>

// Nodo del árbol de Huffman
struct HuffmanNode {
    unsigned char data;       // El carácter (solo relevante en hojas)
    unsigned int frequency;   // Frecuencia de aparición
    HuffmanNode* left;        // Hijo izquierdo
    HuffmanNode* right;       // Hijo derecho
    
    // Constructor para nodos hoja (con carácter)
    HuffmanNode(unsigned char d, unsigned int f) 
        : data(d), frequency(f), left(nullptr), right(nullptr) {}
    
    // Constructor para nodos internos (sin carácter)
    HuffmanNode(unsigned int f, HuffmanNode* l, HuffmanNode* r)
        : data(0), frequency(f), left(l), right(r) {}
    
    // Verificar si es una hoja
    bool is_leaf() const {
        return (left == nullptr && right == nullptr);
    }
};

// Comparador para la cola de prioridad (min-heap por frecuencia)
struct CompareNode {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        // Retorna true si a tiene mayor frecuencia que b
        // Esto hace que el nodo con menor frecuencia esté al frente
        return a->frequency > b->frequency;
    }
};

// Clase para la compresión/descompresión Huffman
class HuffmanCoder {
private:
    HuffmanNode* root;  // Raíz del árbol de Huffman
    std::map<unsigned char, std::string> huffman_codes;  // Mapeo: carácter -> código
    
    // Función auxiliar para liberar memoria del árbol
    void delete_tree(HuffmanNode* node) {
        if (node == nullptr) return;
        delete_tree(node->left);
        delete_tree(node->right);
        delete node;
    }
    
    // Función recursiva para generar los códigos Huffman
    // Recorre el árbol: izquierda = '0', derecha = '1'
    void generate_codes(HuffmanNode* node, std::string code) {
        if (node == nullptr) return;
        
        // Si es una hoja, guardar el código para ese carácter
        if (node->is_leaf()) {
            huffman_codes[node->data] = code;
            return;
        }
        
        // Recorrer recursivamente: izquierda con '0', derecha con '1'
        generate_codes(node->left, code + "0");
        generate_codes(node->right, code + "1");
    }
    
    // Construir el árbol de Huffman a partir de las frecuencias
    HuffmanNode* build_tree(const std::map<unsigned char, unsigned int>& frequencies) {
        // Cola de prioridad (min-heap) para construir el árbol
        std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, CompareNode> pq;
        
        // Crear un nodo hoja para cada carácter y agregarlo a la cola
        for (const auto& pair : frequencies) {
            pq.push(new HuffmanNode(pair.first, pair.second));
        }
        
        // Caso especial: si solo hay un carácter único
        if (pq.size() == 1) {
            HuffmanNode* single = pq.top();
            pq.pop();
            // Crear un nodo padre artificial para evitar código vacío
            return new HuffmanNode(single->frequency, single, nullptr);
        }
        
        // Construir el árbol combinando los dos nodos de menor frecuencia
        while (pq.size() > 1) {
            // Extraer los dos nodos con menor frecuencia
            HuffmanNode* left = pq.top();
            pq.pop();
            HuffmanNode* right = pq.top();
            pq.pop();
            
            // Crear un nuevo nodo interno con la suma de frecuencias
            unsigned int sum_freq = left->frequency + right->frequency;
            HuffmanNode* parent = new HuffmanNode(sum_freq, left, right);
            
            // Insertar el nuevo nodo en la cola
            pq.push(parent);
        }
        
        // El último nodo en la cola es la raíz del árbol
        return pq.top();
    }
    
    // Serializar el árbol para guardarlo en el archivo comprimido
    // Usamos preorden: (tipo)(data_si_es_hoja)(izquierdo)(derecho)
    void serialize_tree(HuffmanNode* node, std::vector<unsigned char>& output) {
        if (node == nullptr) return;
        
        if (node->is_leaf()) {
            // Marcador '1' indica que es hoja
            output.push_back(1);
            // Guardar el carácter
            output.push_back(node->data);
        } else {
            // Marcador '0' indica que es nodo interno
            output.push_back(0);
            // Serializar recursivamente los hijos
            serialize_tree(node->left, output);
            serialize_tree(node->right, output);
        }
    }
    
    // Deserializar el árbol desde el archivo comprimido
    HuffmanNode* deserialize_tree(const std::vector<unsigned char>& data, size_t& index) {
        if (index >= data.size()) return nullptr;
        
        unsigned char marker = data[index++];
        
        if (marker == 1) {
            // Es una hoja
            if (index >= data.size()) return nullptr;
            unsigned char ch = data[index++];
            return new HuffmanNode(ch, 0);
        } else {
            // Es un nodo interno
            HuffmanNode* left = deserialize_tree(data, index);
            HuffmanNode* right = deserialize_tree(data, index);
            return new HuffmanNode(0, left, right);
        }
    }
    
public:
    // Constructor
    HuffmanCoder() : root(nullptr) {}
    
    // Destructor
    ~HuffmanCoder() {
        delete_tree(root);
    }
    
    // COMPRIMIR: Convierte datos originales en datos comprimidos
    std::vector<unsigned char> compress(const std::vector<unsigned char>& input) {
        std::vector<unsigned char> output;
        
        // Caso especial: entrada vacía
        if (input.empty()) {
            return output;
        }
        
        // PASO 1: Calcular frecuencias de cada byte
        std::map<unsigned char, unsigned int> frequencies;
        for (unsigned char byte : input) {
            frequencies[byte]++;
        }
        
        std::cout << "  → Frecuencias calculadas para " << frequencies.size() 
                  << " símbolos únicos\n";
        
        // PASO 2: Construir el árbol de Huffman
        root = build_tree(frequencies);
        
        // PASO 3: Generar los códigos Huffman
        huffman_codes.clear();
        generate_codes(root, "");
        
        std::cout << "  → Códigos Huffman generados\n";
        
        // Mostrar algunos códigos (solo para debug)
        if (huffman_codes.size() <= 10) {
            for (const auto& pair : huffman_codes) {
                std::cout << "     '" << (char)pair.first << "' -> " 
                          << pair.second << "\n";
            }
        }
        
        // PASO 4: Serializar el árbol en el output
        std::vector<unsigned char> tree_data;
        serialize_tree(root, tree_data);
        
        // Guardar el tamaño del árbol serializado (4 bytes)
        unsigned int tree_size = tree_data.size();
        output.push_back((tree_size >> 24) & 0xFF);
        output.push_back((tree_size >> 16) & 0xFF);
        output.push_back((tree_size >> 8) & 0xFF);
        output.push_back(tree_size & 0xFF);
        
        // Agregar el árbol serializado
        output.insert(output.end(), tree_data.begin(), tree_data.end());
        
        // PASO 5: Codificar los datos usando los códigos Huffman
        std::string encoded_bits;
        for (unsigned char byte : input) {
            encoded_bits += huffman_codes[byte];
        }
        
        // Guardar la cantidad de bits válidos en el último byte
        unsigned char padding = (8 - (encoded_bits.length() % 8)) % 8;
        output.push_back(padding);
        
        // Convertir la cadena de bits a bytes
        for (size_t i = 0; i < encoded_bits.length(); i += 8) {
            unsigned char byte = 0;
            for (int j = 0; j < 8 && i + j < encoded_bits.length(); j++) {
                if (encoded_bits[i + j] == '1') {
                    byte |= (1 << (7 - j));
                }
            }
            output.push_back(byte);
        }
        
        std::cout << "  → Compresión completada: " << input.size() 
                  << " bytes → " << output.size() << " bytes\n";
        std::cout << "  → Ratio: " << (100.0 * output.size() / input.size()) 
                  << "%\n";
        
        return output;
    }
    
    // DESCOMPRIMIR: Convierte datos comprimidos en datos originales
    std::vector<unsigned char> decompress(const std::vector<unsigned char>& input) {
        std::vector<unsigned char> output;
        
        if (input.size() < 5) {
            std::cerr << "Error: Archivo comprimido demasiado pequeño\n";
            return output;
        }
        
        size_t index = 0;
        
        // PASO 1: Leer el tamaño del árbol serializado
        unsigned int tree_size = 0;
        tree_size |= ((unsigned int)input[index++] << 24);
        tree_size |= ((unsigned int)input[index++] << 16);
        tree_size |= ((unsigned int)input[index++] << 8);
        tree_size |= ((unsigned int)input[index++]);
        
        if (index + tree_size >= input.size()) {
            std::cerr << "Error: Tamaño de árbol inválido\n";
            return output;
        }
        
        // PASO 2: Deserializar el árbol
        delete_tree(root);
        root = deserialize_tree(input, index);
        
        if (root == nullptr) {
            std::cerr << "Error: No se pudo reconstruir el árbol\n";
            return output;
        }
        
        std::cout << "  → Árbol de Huffman reconstruido\n";
        
        // PASO 3: Leer el padding
        unsigned char padding = input[index++];
        
        // PASO 4: Decodificar los datos
        HuffmanNode* current = root;
        
        // Procesar cada byte de datos comprimidos
        for (size_t i = index; i < input.size(); i++) {
            unsigned char byte = input[i];
            
            // Procesar cada bit del byte
            int bits_to_process = 8;
            // En el último byte, ignorar el padding
            if (i == input.size() - 1) {
                bits_to_process = 8 - padding;
            }
            
            for (int j = 0; j < bits_to_process; j++) {
                // Leer el bit más significativo
                bool bit = (byte & (1 << (7 - j))) != 0;
                
                // Navegar por el árbol: 0=izquierda, 1=derecha
                if (bit) {
                    current = current->right;
                } else {
                    current = current->left;
                }
                
                // Si llegamos a una hoja, tenemos un carácter completo
                if (current->is_leaf()) {
                    output.push_back(current->data);
                    current = root;  // Volver a la raíz
                }
            }
        }
        
        std::cout << "  → Descompresión completada: " << input.size() 
                  << " bytes → " << output.size() << " bytes\n";
        
        return output;
    }
};

#endif // HUFFMAN_H