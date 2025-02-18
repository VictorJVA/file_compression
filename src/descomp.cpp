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

void generarCodigos(Nodo* raiz, string codigo, unordered_map<char, string>& codigos, unordered_map<string, char>& decodificacion) {
    if (!raiz) return;
    if (raiz->caracter != '\0') {
        codigos[raiz->caracter] = codigo;
        decodificacion[codigo] = raiz->caracter;
    }
    generarCodigos(raiz->izquierda, codigo + "1", codigos, decodificacion);
    generarCodigos(raiz->derecha, codigo + "0", codigos, decodificacion);
}

unordered_map<char, int> calcularFrecuencia(const string& contenido) {
    unordered_map<char, int> frecuencias;
    for (char c : contenido)
        frecuencias[c]++;
    return frecuencias;
}

string leerArchivo(const char* nombreArchivo) {
    ifstream archivo(nombreArchivo, ios::binary);
    if (!archivo) {
        cerr << "Error al abrir el archivo" << endl;
        exit(EXIT_FAILURE);
    }
    string contenido((istreambuf_iterator<char>(archivo)), istreambuf_iterator<char>());
    archivo.close();
    return contenido;
}

void escribirArchivo(const char* nombreArchivo, const string& contenido) {
    ofstream archivo(nombreArchivo, ios::binary);
    if (!archivo) {
        cerr << "Error al escribir el archivo" << endl;
        exit(EXIT_FAILURE);
    }
    archivo << contenido;
    archivo.close();
}

void guardarCodigos(const unordered_map<char, string>& codigos, const string& nombreArchivo) {
    ofstream archivo(nombreArchivo);
    if (!archivo) {
        cerr << "Error al escribir el archivo de códigos" << endl;
        exit(EXIT_FAILURE);
    }
    for (const auto& par : codigos) {
        archivo << par.first << " " << par.second << "\n";
    }
    archivo.close();
}

string descomprimir(const string& contenidoComprimido, const unordered_map<string, char>& decodificacion) {
    string resultado, temp;
    for (char bit : contenidoComprimido) {
        temp += bit;
        if (decodificacion.count(temp)) {
            resultado += decodificacion.at(temp);
            temp.clear();
        }
    }
    return resultado;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " -c <archivo> | -h <archivo.huff>" << endl;
        return EXIT_FAILURE;
    }
    string opcion = argv[1];
    string nombreArchivo = argv[2];

    if (opcion == "-c") {
        string contenido = leerArchivo(nombreArchivo.c_str());
        unordered_map<char, int> frecuencias = calcularFrecuencia(contenido);
        Nodo* raiz = construirArbolHuffman(frecuencias);
        unordered_map<char, string> codigos;
        unordered_map<string, char> decodificacion;
        generarCodigos(raiz, "", codigos, decodificacion);

        string contenidoComprimido;
        for (char c : contenido)
            contenidoComprimido += codigos[c];

        escribirArchivo((nombreArchivo + ".huff").c_str(), contenidoComprimido);
        guardarCodigos(codigos, (nombreArchivo + ".codigos").c_str());
        cout << "Archivo comprimido generado: " << nombreArchivo << ".huff" << endl;
        cout << "Códigos almacenados en: " << nombreArchivo << ".codigos" << endl;
    } else if (opcion == "-h") {
        ifstream archivoCodigos(nombreArchivo.substr(0, nombreArchivo.find_last_of('.')) + ".codigos");
        if (!archivoCodigos) {
            cerr << "Error al abrir el archivo de códigos" << endl;
            return EXIT_FAILURE;
        }
        unordered_map<string, char> decodificacion;
        char caracter;
        string codigo;
        while (archivoCodigos >> noskipws >> caracter >> codigo) {
            decodificacion[codigo] = caracter;
        }
        archivoCodigos.close();

        string contenidoComprimido = leerArchivo(nombreArchivo.c_str());
        string contenidoOriginal = descomprimir(contenidoComprimido, decodificacion);
        escribirArchivo((nombreArchivo.substr(0, nombreArchivo.find_last_of('.')) + "_descomprimido.txt").c_str(), contenidoOriginal);
        cout << "Archivo descomprimido generado." << endl;
    } else {
        cerr << "Opción no válida. Use -c para comprimir o -h para descomprimir." << endl;
        return EXIT_FAILURE;
    }
    return 0;
}
