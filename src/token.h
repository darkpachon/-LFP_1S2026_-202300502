#ifndef TOKEN_H
#define TOKEN_H
#include <string>

enum class TokenType {
    // Palabras reservadas
    TABLERO, COLUMNA, TAREA, PRIORIDAD, RESPONSABLE, FECHA_LIMITE,
    // Prioridades
    ALTA, MEDIA, BAJA,
    // Literales
    CADENA, FECHA, ENTERO,
    // Delimitadores
    LLAVE_IZQ, LLAVE_DER, CORCHETE_IZQ, CORCHETE_DER, DOS_PUNTOS, COMA, PUNTO_COMA,
    // Fin de archivo
    END_OF_FILE,
    // Token de error léxico (carácter no reconocido, etc.)
    ERROR_LEX
};

struct Token {
    TokenType tipo;
    std::string lexema;
    int linea;
    int columna; // columna del primer carácter del token
};
#endif