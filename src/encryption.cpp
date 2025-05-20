#include "encryption.h"
#include <fstream>
#include <random>
#include <algorithm>

std::string xorEncrypt(const std::string &data, const std::string &key)
{
    std::string result;
    result.reserve(data.length());

    for (size_t i = 0; i < data.length(); ++i)
    {
        result += data[i] ^ key[i % key.length()];
    }

    return result;
}

std::string generateKey(size_t length)
{
    std::string key;
    key.reserve(length);

    // Generador de números aleatorios
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    // Generar clave aleatoria
    for (size_t i = 0; i < length; ++i)
    {
        key += static_cast<char>(dis(gen));
    }

    return key;
}

void saveKey(const std::string &key, const std::string &filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("No se pudo abrir el archivo para guardar la clave");
    }
    file.write(key.c_str(), key.length());
}

std::string loadKey(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("No se pudo abrir el archivo para cargar la clave");
    }

    std::string key;
    file.seekg(0, std::ios::end);
    key.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&key[0], key.size());

    return key;
}