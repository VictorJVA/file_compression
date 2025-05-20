#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <filesystem>
#include <omp.h>
#include "encryption.h"
using namespace std;

struct Nodo
{
    char caracter;
    int frecuencia;
    Nodo *izquierda;
    Nodo *derecha;
    Nodo(char c, int f, Nodo *izq = nullptr, Nodo *der = nullptr)
        : caracter(c), frecuencia(f), izquierda(izq), derecha(der) {}
};

struct Comparar
{
    bool operator()(Nodo *a, Nodo *b)
    {
        return a->frecuencia > b->frecuencia;
    }
};

Nodo *construirArbolHuffman(const unordered_map<char, int> &frecuencias)
{
    priority_queue<Nodo *, vector<Nodo *>, Comparar> cola;
    for (auto &par : frecuencias)
        cola.push(new Nodo(par.first, par.second));
    while (cola.size() > 1)
    {
        Nodo *izquierda = cola.top();
        cola.pop();
        Nodo *derecha = cola.top();
        cola.pop();
        Nodo *padre = new Nodo('\0', izquierda->frecuencia + derecha->frecuencia, izquierda, derecha);
        cola.push(padre);
    }
    return cola.top();
}

void generarCodigos(Nodo *raiz, string codigo, unordered_map<char, string> &codigos, unordered_map<string, char> &decodificacion)
{
    if (!raiz)
        return;
    if (raiz->caracter != '\0')
    {
        codigos[raiz->caracter] = codigo;
        decodificacion[codigo] = raiz->caracter;
    }
    generarCodigos(raiz->izquierda, codigo + "1", codigos, decodificacion);
    generarCodigos(raiz->derecha, codigo + "0", codigos, decodificacion);
}

unordered_map<char, int> calcularFrecuencia(const string &contenido)
{
    vector<unordered_map<char, int>> frecuencias_local(omp_get_max_threads());
#pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t n = contenido.length();
#pragma omp for
        for (size_t i = 0; i < n; ++i)
        {
            frecuencias_local[thread_id][contenido[i]]++;
        }
    }
    unordered_map<char, int> frecuencias;
    for (const auto &local_map : frecuencias_local)
    {
        for (const auto &par : local_map)
        {
            frecuencias[par.first] += par.second;
        }
    }
    return frecuencias;
}

string leerArchivo(const char *nombreArchivo)
{
    ifstream archivo(nombreArchivo, ios::binary);
    if (!archivo)
    {
        cerr << "Error al abrir el archivo" << endl;
        exit(EXIT_FAILURE);
    }
    string contenido((istreambuf_iterator<char>(archivo)), istreambuf_iterator<char>());
    archivo.close();
    return contenido;
}

void escribirArchivo(const char *nombreArchivo, const string &contenido)
{
    ofstream archivo(nombreArchivo, ios::binary);
    if (!archivo)
    {
        cerr << "Error al escribir el archivo" << endl;
        exit(EXIT_FAILURE);
    }
    archivo << contenido;
    archivo.close();
}

void guardarCodigos(const unordered_map<char, string> &codigos, const string &nombreArchivo)
{
    ofstream archivo(nombreArchivo);
    if (!archivo)
    {
        cerr << "Error al escribir el archivo de códigos" << endl;
        exit(EXIT_FAILURE);
    }
    for (const auto &par : codigos)
    {
        archivo << static_cast<int>(par.first) << " " << par.second << "\n";
    }
    archivo.close();
}

string comprimirContenido(const string &contenido, const unordered_map<char, string> &codigos)
{
    vector<string> resultados_local(omp_get_max_threads());
#pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t n = contenido.length();
#pragma omp for
        for (size_t i = 0; i < n; ++i)
        {
            resultados_local[thread_id] += codigos.at(contenido[i]);
        }
    }
    string resultado;
    for (const auto &local_str : resultados_local)
    {
        resultado += local_str;
    }
    return resultado;
}

// DESCOMPRESIÓN SECUENCIAL
string descomprimir(const string &contenidoComprimido, const unordered_map<string, char> &decodificacion)
{
    string resultado, temp;
    for (char bit : contenidoComprimido)
    {
        temp += bit;
        if (decodificacion.count(temp))
        {
            resultado += decodificacion.at(temp);
            temp.clear();
        }
    }
    return resultado;
}

bool verificarArchivo(const string &nombreArchivo, const string &tipo)
{
    ifstream archivo(nombreArchivo);
    if (!archivo)
    {
        cerr << "Error: No se puede abrir el archivo " << nombreArchivo << endl;
        cerr << "Tipo de archivo: " << tipo << endl;
        cerr << "Asegúrese de que el archivo existe y tiene los permisos correctos." << endl;
        return false;
    }
    return true;
}

string obtenerNombreBase(const string &nombreArchivo)
{
    size_t pos = nombreArchivo.find_last_of('.');
    if (pos == string::npos)
    {
        return nombreArchivo;
    }
    return nombreArchivo.substr(0, pos);
}

int main(int argc, char *argv[])
{
    // Configurar el número de hilos de OpenMP
    int num_threads = omp_get_max_threads();
    omp_set_num_threads(num_threads);
    cout << "Usando " << num_threads << " hilos para el procesamiento" << endl;

    if (argc != 3 && argc != 4)
    {
        cerr << "Uso: " << argv[0] << " -c <archivo> [-e] | -h <archivo.huff> [-e]" << endl;
        cerr << "  -e: opcional, habilita la encriptación" << endl;
        return EXIT_FAILURE;
    }

    string opcion = argv[1];
    string nombreArchivo = argv[2];
    bool encriptar = (argc == 4 && string(argv[3]) == "-e");

    if (opcion == "-c")
    {
        if (!verificarArchivo(nombreArchivo, "archivo a comprimir"))
        {
            return EXIT_FAILURE;
        }

        string contenido = leerArchivo(nombreArchivo.c_str());

        // Encriptar si se solicita
        string key;
        if (encriptar)
        {
            key = generateKey(32);
            contenido = xorEncrypt(contenido, key);
            saveKey(key, nombreArchivo + ".key");
            cout << "Clave guardada en: " << nombreArchivo << ".key" << endl;
        }

        // Medir tiempo de compresión
        double start_time = omp_get_wtime();

        unordered_map<char, int> frecuencias = calcularFrecuencia(contenido);
        Nodo *raiz = construirArbolHuffman(frecuencias);
        unordered_map<char, string> codigos;
        unordered_map<string, char> decodificacion;
        generarCodigos(raiz, "", codigos, decodificacion);

        string contenidoComprimido = comprimirContenido(contenido, codigos);

        double end_time = omp_get_wtime();
        cout << "Tiempo de compresión: " << (end_time - start_time) << " segundos" << endl;

        string nombreBase = obtenerNombreBase(nombreArchivo);
        escribirArchivo((nombreBase + ".huff").c_str(), contenidoComprimido);
        guardarCodigos(codigos, (nombreBase + ".codigos").c_str());
        cout << "Archivo comprimido generado: " << nombreBase << ".huff" << endl;
        cout << "Códigos almacenados en: " << nombreBase << ".codigos" << endl;
    }
    else if (opcion == "-h")
    {
        // Verificar que el archivo tiene la extensión correcta
        if (nombreArchivo.find(".huff") == string::npos)
        {
            cerr << "Error: El archivo debe tener la extensión .huff" << endl;
            return EXIT_FAILURE;
        }

        string nombreBase = obtenerNombreBase(nombreArchivo);
        string rutaCodigos = nombreBase + ".codigos";

        if (!verificarArchivo(rutaCodigos, "archivo de códigos"))
        {
            cerr << "Asegúrese de que el archivo de códigos existe y está en el mismo directorio que el archivo comprimido." << endl;
            return EXIT_FAILURE;
        }

        ifstream archivoCodigos(rutaCodigos);
        unordered_map<string, char> decodificacion;
        int caracter;
        string codigo;

        while (archivoCodigos >> caracter >> codigo)
        {
            decodificacion[codigo] = static_cast<char>(caracter);
        }
        archivoCodigos.close();

        if (decodificacion.empty())
        {
            cerr << "Error: El archivo de códigos está vacío o tiene un formato incorrecto" << endl;
            return EXIT_FAILURE;
        }

        // Medir tiempo de descompresión
        double start_time = omp_get_wtime();

        string contenidoComprimido = leerArchivo(nombreArchivo.c_str());
        string contenidoOriginal = descomprimir(contenidoComprimido, decodificacion);

        double end_time = omp_get_wtime();
        cout << "Tiempo de descompresión: " << (end_time - start_time) << " segundos" << endl;

        // Desencriptar si está encriptado
        if (encriptar)
        {
            string archivoClave = nombreBase + ".key";
            if (!verificarArchivo(archivoClave, "archivo de clave"))
            {
                cerr << "Error: No se encontró el archivo de clave para desencriptar" << endl;
                return EXIT_FAILURE;
            }

            try
            {
                string key = loadKey(archivoClave);
                contenidoOriginal = xorEncrypt(contenidoOriginal, key);
            }
            catch (const std::exception &e)
            {
                cerr << "Error al cargar la clave: " << e.what() << endl;
                return EXIT_FAILURE;
            }
        }

        string archivoDescomprimido = nombreBase + "_descomprimido.txt";
        escribirArchivo(archivoDescomprimido.c_str(), contenidoOriginal);
        cout << "Archivo descomprimido generado: " << archivoDescomprimido << endl;
    }
    else
    {
        cerr << "Opción no válida. Use -c para comprimir o -h para descomprimir." << endl;
        return EXIT_FAILURE;
    }
    return 0;
}
