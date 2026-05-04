#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H
#include "lexicalanalyzer.h"
#include "errormanager.h"
#include <vector>
#include <string>

struct NodoAST {
    std::string etiqueta;  // nombre de producción o tipo de token
    std::string valor;     // lexema (para hojas)
    std::vector<NodoAST*> hijos;
    NodoAST(const std::string& e, const std::string& v = "") : etiqueta(e), valor(v) {}
};

class SyntaxAnalyzer {
public:
    SyntaxAnalyzer(LexicalAnalyzer& lex, ErrorManager& err);
    NodoAST* parse();       // retorna la raíz del árbol, nullptr si error fatal
    bool exito() const { return !errorManager.hayErrores(); }
private:
    LexicalAnalyzer& lexer;
    ErrorManager& errorManager;
    Token tokenActual;

    void avanzar();
    void match(TokenType esperado, const std::string& nombreEsperado);
    void sincronizar();     // recuperación de errores

    NodoAST* programa();
    NodoAST* listaColumnas();
    NodoAST* columna();
    NodoAST* listaTareas();
    NodoAST* tarea();
    NodoAST* listaAtributos();
    NodoAST* atributo();
    NodoAST* prioridad();
};

#endif