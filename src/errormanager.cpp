#include "errormanager.h"

void ErrorManager::agregarErrorLexico(const Token& token, const std::string& desc) {
    errores.push_back({(int)errores.size()+1, token.lexema, "Léxico", desc, token.linea, token.columna});
}

void ErrorManager::agregarErrorSintactico(const Token& token, const std::string& esperado) {
    std::string desc = "Se esperaba " + esperado + ", se encontró '" + token.lexema + "'";
    errores.push_back({(int)errores.size()+1, token.lexema, "Sintáctico", desc, token.linea, token.columna});
}