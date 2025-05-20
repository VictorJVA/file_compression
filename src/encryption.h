#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>
#include <vector>

// Función para encriptar/desencriptar usando XOR con una clave
std::string xorEncrypt(const std::string &data, const std::string &key);

// Función para generar una clave aleatoria
std::string generateKey(size_t length);

// Función para guardar la clave en un archivo
void saveKey(const std::string &key, const std::string &filename);

// Función para cargar la clave desde un archivo
std::string loadKey(const std::string &filename);

#endif // ENCRYPTION_H