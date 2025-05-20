# Compresor y Encriptador de Archivos con Huffman y OpenMP

Este proyecto implementa un compresor y descompresor de archivos basado en el algoritmo de Huffman, con opción de encriptación XOR y aceleración mediante OpenMP para procesamiento paralelo.

## Requisitos
- **C++17** o superior
- **g++** (recomendado)
- **OpenMP** (generalmente incluido en g++)

## Clonación del repositorio
```bash
git clone <URL_DEL_REPOSITORIO>
cd <nombre_del_repositorio>
```

## Compilación
Compila el proyecto con soporte para OpenMP:
```bash
g++ -o compresor src/descomp.cpp src/encryption.cpp -std=c++17 -fopenmp
```

## Uso
### Compresión
```bash
./compresor -c <archivo.txt>
```
Esto generará dos archivos:
- `<archivo>.huff` (archivo comprimido)
- `<archivo>.codigos` (códigos de Huffman)

### Compresión con encriptación
```bash
./compresor -c <archivo.txt> -e
```
Esto además generará:
- `<archivo>.key` (clave de encriptación)

### Descompresión
```bash
./compresor -h <archivo.huff>
```
Esto generará:
- `<archivo>_descomprimido.txt` (archivo descomprimido)

### Descompresión con desencriptado
```bash
./compresor -h <archivo.huff> -e
```
Asegúrate de que el archivo `.key` esté presente en el mismo directorio.

## Notas sobre OpenMP
- El programa detecta automáticamente el número de hilos disponibles y paraleliza el cálculo de frecuencias y la compresión.
- La descompresión se realiza de forma secuencial para garantizar la integridad de los datos.
- Verás en consola el número de hilos usados y el tiempo de procesamiento para compresión y descompresión.

## Ejemplo rápido
```bash
# Crear un archivo de prueba
 echo "BANANA" > prueba.txt

# Comprimir
 ./compresor -c prueba.txt

# Descomprimir
 ./compresor -h prueba.huff

# Comprimir y encriptar
 ./compresor -c prueba.txt -e

# Descomprimir y desencriptar
 ./compresor -h prueba.huff -e
```

## Autor
- [Tu Nombre] 