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
  string buffer;
    string resultado;
    for (char bit : contenidoComprimido) {
        buffer += bit;
        auto it = decodificacion.find(buffer);
        if (it != decodificacion.end()) {
            resultado += it->second;
            buffer.clear();
        }
    }
    if (!buffer.empty()) {
        cerr << "Error: Bits residuales no decodificados." << endl;
    }
    return resultado;
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

void comprimirArchivo(string nombreArchivo){
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
}
void descomprimirArchivo(string nombreArchivo){
  string nombreBase = nombreArchivo.substr(0, nombreArchivo.find_last_of('.'));
    ifstream archivoCodigos(nombreBase + ".codigos");
    if (!archivoCodigos) {
        cerr << "Error al abrir el archivo de códigos" << endl;
        //return EXIT_FAILURE;
    }

    unordered_map<string, char> decodificacion;
    string linea;
    while (getline(archivoCodigos, linea)) {
        if (linea.empty()) continue;
        
        // Separar el carácter y su código
        size_t espacio = linea.find(' ');
        if (espacio == string::npos || espacio == linea.size() - 1) {
            cerr << "Formato inválido en línea: " << linea << endl;
            exit(EXIT_FAILURE);
        }
        
        char caracter = linea[0];
        string codigo = linea.substr(espacio + 1);
        
        decodificacion[codigo] = caracter;
    }
    archivoCodigos.close();
      archivoCodigos.close();
      string contenidoComprimido = leerArchivo(nombreArchivo.c_str());
      string contenidoOriginal = descomprimir(contenidoComprimido, decodificacion);
      escribirArchivo((nombreArchivo.substr(0, nombreArchivo.find_last_of('.')) + "_descomprimido.txt").c_str(), contenidoOriginal);
      cout << "Archivo descomprimido generado." << endl;
}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " -c <archivo>" << endl;
        return EXIT_FAILURE;
    }
    string opcion = argv[1];
    if (opcion == "--version" || opcion == "-v") {
        cout << "Versión 1.0" << endl;
        return 0;
    }
    if (opcion == "--help" || opcion == "-h") {
        cout << "Uso: " << argv[0] << " [opciones] <archivo>" << endl;
        cout << "\nOpciones:" << endl;
        cout << "  -c <archivo>   Comprime el archivo especificado utilizando el algoritmo de Huffman." << endl;
        cout << "  -d <archivo>   Descomprime un archivo previamente comprimido con este programa." << endl;
        cout << "  -h, --help     Muestra esta ayuda." << endl;
        cout << "  -v, --version  Muestra la versión del programa." << endl;
        cout << "\nEjemplo:" << endl;
        cout << "  " << argv[0] << " -c documento.txt    # Comprime 'texto_ejemplo' a 'documento.txt.huff'" << endl;
        cout << "  " << argv[0] << " -d documento.txt.huff  # Descomprime 'documento.txt.huff' a 'documento_descomprimido.txt'" << endl;
        return 0;
    }
    
    string nombreArchivo = argv[2];
    if (opcion == "-c" || opcion == "--compress") {
        if (argc < 3) {
            cerr << "Falta el nombre del archivo" << endl;
            return EXIT_FAILURE;
        }
      comprimirArchivo(nombreArchivo);
  } else if (opcion == "-d" || opcion == "descompress") {
    descomprimirArchivo(nombreArchivo);

  }
     else {
      cerr << "Opción no válida. Use -c para comprimir o -h para descomprimir." << endl;
      return EXIT_FAILURE;
  }
    return 0;
}

