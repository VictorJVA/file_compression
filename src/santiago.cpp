#include "huffman.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

// Implementación del constructor de Nodo
Nodo::Nodo(char c, int f, Nodo* izq, Nodo* der)
    : caracter(c), frecuencia(f), izquierda(izq), derecha(der) {}

// Implementación de la estructura Comparar
bool Comparar::operator()(Nodo* a, Nodo* b) {
    return a->frecuencia > b->frecuencia;
}

// Función para construir el árbol de Huffman
Nodo* construirArbolHuffman(const unordered_map<char, int>& frecuencias) {
    priority_queue<Nodo*, vector<Nodo*>, Comparar> cola;
    for (auto& par : frecuencias)
        cola.push(new Nodo(par.first, par.second));
    while (cola.size() > 1) {
        Nodo* izquierda = cola.top(); cola.pop();
        Nodo* derecha = cola.top(); cola.pop();
        Nodo* padre = new Nodo('\0', izquierda->frecuencia + derecha->frecuencia, izquierda, derecha);
        cola.push(padre);
    }
    return cola.top();
}

// Función para generar los códigos Huffman
void generarCodigos(Nodo* raiz, string codigo, unordered_map<char, string>& codigos) {
    if (!raiz) return;
    if (raiz->caracter != '\0')
        codigos[raiz->caracter] = codigo;
    generarCodigos(raiz->izquierda, codigo + "0", codigos);
    generarCodigos(raiz->derecha, codigo + "1", codigos);
}

// Función para calcular la frecuencia de los caracteres
unordered_map<char, int> calcularFrecuencia(const string& contenido) {
    unordered_map<char, int> frecuencias;
    for (char c : contenido)
        frecuencias[c]++;
    return frecuencias;
}

// Función para leer el contenido de un archivo
string leerArchivo(const char* nombreArchivo) {
    int fd = open(nombreArchivo, O_RDONLY);
    if (fd == -1) {
        cerr << "Error al abrir el archivo" << endl;
        exit(EXIT_FAILURE);
    }
    char buffer[1024];
    string contenido;
    ssize_t bytesLeidos;
    while ((bytesLeidos = read(fd, buffer, sizeof(buffer))) > 0)
        contenido.append(buffer, bytesLeidos);
    close(fd);
    return contenido;
}

// Función para escribir el contenido en un archivo
void escribirArchivo(const char* nombreArchivo, const string& contenido) {
    int fd = open(nombreArchivo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        cerr << "Error al escribir el archivo" << endl;
        exit(EXIT_FAILURE);
    }
    write(fd, contenido.c_str(), contenido.size());
    close(fd);
}

// Función para serializar el árbol de Huffman
string serializarArbol(Nodo* raiz) {
    if (!raiz) return "";
    if (raiz->caracter != '\0') {
        return "1" + string(1, raiz->caracter); // '1' indica un nodo hoja
    }
    return "0" + serializarArbol(raiz->izquierda) + serializarArbol(raiz->derecha); // '0' indica un nodo interno
}

// Función para deserializar el árbol de Huffman
Nodo* deserializarArbol(const string& arbolSerializado, size_t& index) {
    if (index >= arbolSerializado.length()) return nullptr;

    char bit = arbolSerializado[index++];
    if (bit == '1') {
        char caracter = arbolSerializado[index++];
        return new Nodo(caracter, 0);
    } else {
        Nodo* izquierda = deserializarArbol(arbolSerializado, index);
        Nodo* derecha = deserializarArbol(arbolSerializado, index);
        return new Nodo('\0', 0, izquierda, derecha);
    }
}

// Función para comprimir un archivo
void comprimirArchivo(const std::string& nombreArchivo) {
    string contenido = leerArchivo(nombreArchivo.c_str());
    unordered_map<char, int> frecuencias = calcularFrecuencia(contenido);
    Nodo* raiz = construirArbolHuffman(frecuencias);
    unordered_map<char, string> codigos;
    generarCodigos(raiz, "", codigos);

    // Serializar el árbol de Huffman
    string arbolSerializado = serializarArbol(raiz);

    // Comprimir el contenido
    string contenidoComprimido;
    for (char c : contenido)
        contenidoComprimido += codigos[c];

    // Escribir el archivo comprimido (árbol + contenido)
    string archivoComprimido = arbolSerializado + "\n" + contenidoComprimido;
    escribirArchivo((nombreArchivo + ".huff").c_str(), archivoComprimido);

    cout << "Archivo comprimido generado: " << nombreArchivo << ".huff" << endl;
}

// Función para descomprimir un archivo
void descomprimirArchivo(const std::string& nombreArchivo) {
    string contenidoComprimido = leerArchivo(nombreArchivo.c_str());

    // Separar el árbol serializado y el contenido comprimido
    size_t separador = contenidoComprimido.find('\n');
    string arbolSerializado = contenidoComprimido.substr(0, separador);
    string bitsComprimidos = contenidoComprimido.substr(separador + 1);

    // Reconstruir el árbol de Huffman
    size_t index = 0;
    Nodo* raiz = deserializarArbol(arbolSerializado, index);

    // Descomprimir el contenido
    string contenidoOriginal;
    Nodo* actual = raiz;
    for (char bit : bitsComprimidos) {
        if (bit == '0') {
            actual = actual->izquierda;
        } else if (bit == '1') {
            actual = actual->derecha;
        }

        if (actual->caracter != '\0') {
            contenidoOriginal += actual->caracter;
            actual = raiz;
        }
    }

    // Escribir el archivo descomprimido
    string nombreDescomprimido = nombreArchivo.substr(0, nombreArchivo.find(".huff")) + "_descomprimido.txt";
    escribirArchivo(nombreDescomprimido.c_str(), contenidoOriginal);

    cout << "Archivo descomprimido generado: " << nombreDescomprimido << endl;
}