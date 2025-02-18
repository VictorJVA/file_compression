#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
using namespace std;

struct Nodo {
    char caracter;
    int frecuencia;
    Nodo* izquierda;
    Nodo* derecha;
    Nodo(char c, int f, Nodo* izq = nullptr, Nodo* der = nullptr)
        : caracter(c), frecuencia(f), izquierda(izq), derecha(der) {}
};

struct Comparar {
    bool operator()(Nodo* a, Nodo* b) {
        return a->frecuencia > b->frecuencia;
    }
};

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

void generarCodigos(Nodo* raiz, string codigo, unordered_map<char, string>& codigos) {
    if (!raiz) return;
    if (raiz->caracter != '\0')
        codigos[raiz->caracter] = codigo;
    generarCodigos(raiz->izquierda, codigo + "1", codigos);
    generarCodigos(raiz->derecha, codigo + "0", codigos);
}

unordered_map<char, int> calcularFrecuencia(const string& contenido) {
    unordered_map<char, int> frecuencias;
    for (char c : contenido)
        frecuencias[c]++;
    return frecuencias;
}

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

void escribirArchivo(const char* nombreArchivo, const string& contenido) {
    int fd = open(nombreArchivo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        cerr << "Error al escribir el archivo" << endl;
        exit(EXIT_FAILURE);
    }
    write(fd, contenido.c_str(), contenido.size());
    close(fd);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " -c <archivo>" << endl;
        return EXIT_FAILURE;
    }
    string opcion = argv[1];
    string nombreArchivo = argv[2];
    string contenido = leerArchivo(nombreArchivo.c_str());
    unordered_map<char, int> frecuencias = calcularFrecuencia(contenido);
    Nodo* raiz = construirArbolHuffman(frecuencias);
    unordered_map<char, string> codigos;
    generarCodigos(raiz, "", codigos);
    string contenidoComprimido;
    for (char c : contenido)
        contenidoComprimido += codigos[c];
    escribirArchivo((nombreArchivo + ".huff").c_str(), contenidoComprimido);
    cout << "Archivo comprimido generado: " << nombreArchivo << ".huff" << endl;
    return 0;
}

