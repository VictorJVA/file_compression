#include <iostream>
#include <string>
#include "huffman.h"

void showHelp() {
    std::cout << "Uso: ./huffman [OPCIÓN] <archivo>\n"
              << "Opciones:\n"
              << "  -h, --help       Muestra este mensaje de ayuda\n"
              << "  -v, --version    Muestra la versión del programa\n"
              << "  -c, --compress   Comprime el archivo indicado\n"
              << "  -x, --descompress Descomprime el archivo indicado\n";
}

void showVersion() {
    std::cout << "huffman v1.0\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        showHelp();
        return 1;
    }

    std::string option = argv[1];
    std::string filename = argv[2];

    if (option == "-h" || option == "--help") {
        showHelp();
    } else if (option == "-v" || option == "--version") {
        showVersion();
    } else if (option == "-c" || option == "--compress") {
        comprimirArchivo(filename);
    } else if (option == "-x" || option == "--descompress") {
        descomprimirArchivo(filename);
    } else {
        std::cerr << "Opción no válida.\n";
        showHelp();
        return 1;
    }

    return 0;
}