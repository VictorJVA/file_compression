#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>
#include <unordered_map>
#include <queue>

// Estructura para el nodo del árbol de Huffman
struct Nodo {
    char caracter;
    int frecuencia;
    Nodo* izquierda;
    Nodo* derecha;
    Nodo(char c, int f, Nodo* izq = nullptr, Nodo* der = nullptr);
};

// Estructura para comparar nodos en la cola de prioridad
struct Comparar {
    bool operator()(Nodo* a, Nodo* b);
};

// Declaraciones de funciones
std::unordered_map<char, int> calcularFrecuencia(const std::string& contenido);
Nodo* construirArbolHuffman(const std::unordered_map<char, int>& frecuencias);
void generarCodigos(Nodo* raiz, std::string codigo, std::unordered_map<char, std::string>& codigos);
std::string leerArchivo(const char* nombreArchivo);
void escribirArchivo(const char* nombreArchivo, const std::string& contenido);
void comprimirArchivo(const std::string& nombreArchivo);
void descomprimirArchivo(const std::string& nombreArchivo);

#endif // HUFFMAN_H