#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H
#include <string>
#include <vector>
#include "token.h"

struct ErrorEntry {
    int numero;
    std::string lexema;
    std::string tipo;       // "Léxico" o "Sintáctico"
    std::string descripcion;
    int linea;
    int columna;
};

class ErrorManager {
public:
    void agregarErrorLexico(const Token& token, const std::string& desc);
    void agregarErrorSintactico(const Token& token, const std::string& esperado);
    const std::vector<ErrorEntry>& getErrores() const { return errores; }
    bool hayErrores() const { return !errores.empty(); }
private:
    std::vector<ErrorEntry> errores;
};

#endif