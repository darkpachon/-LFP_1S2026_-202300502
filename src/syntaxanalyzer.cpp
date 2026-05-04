#include "syntaxanalyzer.h"
#include <iostream>

SyntaxAnalyzer::SyntaxAnalyzer(LexicalAnalyzer& lex, ErrorManager& err)
    : lexer(lex), errorManager(err)
{
    tokenActual = lexer.nextToken(); // primer token
}

void SyntaxAnalyzer::avanzar() {
    tokenActual = lexer.nextToken();
}

void SyntaxAnalyzer::match(TokenType esperado, const std::string& nombreEsperado) {
    if (tokenActual.tipo == esperado) {
        avanzar();
    } else {
        errorManager.agregarErrorSintactico(tokenActual, nombreEsperado);
        sincronizar();
    }
}

void SyntaxAnalyzer::sincronizar() {
    // Avanzar hasta encontrar un token de sincronización
    while (tokenActual.tipo != TokenType::PUNTO_COMA &&
           tokenActual.tipo != TokenType::LLAVE_DER &&
           tokenActual.tipo != TokenType::CORCHETE_DER &&
           tokenActual.tipo != TokenType::TABLERO &&
           tokenActual.tipo != TokenType::COLUMNA &&
           tokenActual.tipo != TokenType::END_OF_FILE)
    {
        avanzar();
    }
    if (tokenActual.tipo != TokenType::END_OF_FILE) {
        avanzar(); // consumir el sincronizador
    }
}

NodoAST* SyntaxAnalyzer::parse() {
    NodoAST* raiz = programa();
    if (tokenActual.tipo != TokenType::END_OF_FILE) {
        // tokens sobrantes, algo no consumido
        errorManager.agregarErrorSintactico(tokenActual, "fin de archivo");
    }
    return raiz;
}

// <programa> ::= TABLERO CADENA "{" <lista_columnas> "}" ";"
NodoAST* SyntaxAnalyzer::programa() {
    NodoAST* nodo = new NodoAST("<programa>");

    // TABLERO
    if (tokenActual.tipo == TokenType::TABLERO) {
        nodo->hijos.push_back(new NodoAST("TABLERO", tokenActual.lexema));
        avanzar();
    } else {
        errorManager.agregarErrorSintactico(tokenActual, "TABLERO");
        sincronizar();
        if (tokenActual.tipo == TokenType::END_OF_FILE) return nodo;
        avanzar(); // intentar continuar
    }

    // CADENA
    if (tokenActual.tipo == TokenType::CADENA) {
        nodo->hijos.push_back(new NodoAST("CADENA", tokenActual.lexema));
        avanzar();
    } else {
        errorManager.agregarErrorSintactico(tokenActual, "cadena de texto");
        sincronizar();
        if (tokenActual.tipo == TokenType::END_OF_FILE) return nodo;
    }

    // "{"
    match(TokenType::LLAVE_IZQ, "{");

    // <lista_columnas>
    nodo->hijos.push_back(listaColumnas());

    // "}"
    match(TokenType::LLAVE_DER, "}");

    // ";"
    match(TokenType::PUNTO_COMA, ";");

    return nodo;
}

// <lista_columnas> ::= <columna> <lista_columnas> | <columna>
NodoAST* SyntaxAnalyzer::listaColumnas() {
    NodoAST* nodo = new NodoAST("<lista_columnas>");
    if (tokenActual.tipo != TokenType::COLUMNA) {
        errorManager.agregarErrorSintactico(tokenActual, "COLUMNA");
        sincronizar();
        return nodo;
    }
    while (tokenActual.tipo == TokenType::COLUMNA) {
        nodo->hijos.push_back(columna());
    }
    return nodo;
}

// <columna> ::= COLUMNA CADENA "{" <lista_tareas> "}" ";"
NodoAST* SyntaxAnalyzer::columna() {
    NodoAST* nodo = new NodoAST("<columna>");

    // COLUMNA
    if (tokenActual.tipo == TokenType::COLUMNA) {
        nodo->hijos.push_back(new NodoAST("COLUMNA", tokenActual.lexema));
        avanzar();
    } else {
        errorManager.agregarErrorSintactico(tokenActual, "COLUMNA");
        sincronizar();
        return nodo;
    }

    // CADENA
    if (tokenActual.tipo == TokenType::CADENA) {
        nodo->hijos.push_back(new NodoAST("CADENA", tokenActual.lexema));
        avanzar();
    } else {
        errorManager.agregarErrorSintactico(tokenActual, "cadena (nombre de columna)");
        sincronizar();
        // intentar continuar si hay '{'
    }

    // "{"
    match(TokenType::LLAVE_IZQ, "{");

    // <lista_tareas>
    nodo->hijos.push_back(listaTareas());

    // "}"
    match(TokenType::LLAVE_DER, "}");

    // ";"
    match(TokenType::PUNTO_COMA, ";");

    return nodo;
}

// <lista_tareas> ::= <tarea> ( "," <tarea> )*
NodoAST* SyntaxAnalyzer::listaTareas() {
    NodoAST* nodo = new NodoAST("<lista_tareas>");
    if (tokenActual.tipo != TokenType::TAREA) {
        errorManager.agregarErrorSintactico(tokenActual, "tarea");
        sincronizar();
        return nodo;
    }

    nodo->hijos.push_back(tarea());
    while (tokenActual.tipo == TokenType::COMA) {
        avanzar();
        if (tokenActual.tipo == TokenType::TAREA) {
            nodo->hijos.push_back(tarea());
        } else {
            errorManager.agregarErrorSintactico(tokenActual, "tarea");
            sincronizar();
            break;
        }
    }
    return nodo;
}

// <tarea> ::= TAREA ":" CADENA "[" <lista_atributos> "]"
NodoAST* SyntaxAnalyzer::tarea() {
    NodoAST* nodo = new NodoAST("<tarea>");
    // TAREA
    if (tokenActual.tipo == TokenType::TAREA) {
        nodo->hijos.push_back(new NodoAST("TAREA", tokenActual.lexema));
        avanzar();
    } else {
        errorManager.agregarErrorSintactico(tokenActual, "tarea");
        sincronizar();
        return nodo;
    }
    // ":"
    match(TokenType::DOS_PUNTOS, ":");
    // CADENA
    if (tokenActual.tipo == TokenType::CADENA) {
        nodo->hijos.push_back(new NodoAST("CADENA", tokenActual.lexema));
        avanzar();
    } else {
        errorManager.agregarErrorSintactico(tokenActual, "cadena (nombre de tarea)");
        sincronizar();
        // intentar continuar si hay '{'
    }
    // "["
    match(TokenType::CORCHETE_IZQ, "[");
    // <lista_atributos>
    nodo->hijos.push_back(listaAtributos());
    // "]"
    match(TokenType::CORCHETE_DER, "]");
    return nodo;
}

// <lista_atributos> ::= <atributo> ( "," <atributo> )*
NodoAST* SyntaxAnalyzer::listaAtributos() {
    NodoAST* nodo = new NodoAST("<lista_atributos>");
    if (tokenActual.tipo != TokenType::PRIORIDAD &&
        tokenActual.tipo != TokenType::RESPONSABLE &&
        tokenActual.tipo != TokenType::FECHA_LIMITE)
    {
        errorManager.agregarErrorSintactico(tokenActual, "prioridad, responsable o fecha_limite");
        sincronizar();
        return nodo;
    }

    nodo->hijos.push_back(atributo());
    while (tokenActual.tipo == TokenType::COMA) {
        avanzar();
        if (tokenActual.tipo == TokenType::PRIORIDAD ||
            tokenActual.tipo == TokenType::RESPONSABLE ||
            tokenActual.tipo == TokenType::FECHA_LIMITE)
        {
            nodo->hijos.push_back(atributo());
        } else {
            errorManager.agregarErrorSintactico(tokenActual, "atributo");
            sincronizar();
            break;
        }
    }
    return nodo;
}

// <atributo> ::= PRIORIDAD ":" <prioridad> | RESPONSABLE ":" CADENA | FECHA_LIMITE ":" FECHA
NodoAST* SyntaxAnalyzer::atributo() {
    NodoAST* nodo = new NodoAST("<atributo>");
    if (tokenActual.tipo == TokenType::PRIORIDAD) {
        nodo->hijos.push_back(new NodoAST("PRIORIDAD", tokenActual.lexema));
        avanzar();
        match(TokenType::DOS_PUNTOS, ":");
        nodo->hijos.push_back(prioridad());
    } else if (tokenActual.tipo == TokenType::RESPONSABLE) {
        nodo->hijos.push_back(new NodoAST("RESPONSABLE", tokenActual.lexema));
        avanzar();
        match(TokenType::DOS_PUNTOS, ":");
        if (tokenActual.tipo == TokenType::CADENA) {
            nodo->hijos.push_back(new NodoAST("CADENA", tokenActual.lexema));
            avanzar();
        } else {
            errorManager.agregarErrorSintactico(tokenActual, "cadena (nombre del responsable)");
            sincronizar();
        }
    } else if (tokenActual.tipo == TokenType::FECHA_LIMITE) {
        nodo->hijos.push_back(new NodoAST("FECHA_LIMITE", tokenActual.lexema));
        avanzar();
        match(TokenType::DOS_PUNTOS, ":");
        if (tokenActual.tipo == TokenType::FECHA) {
            nodo->hijos.push_back(new NodoAST("FECHA", tokenActual.lexema));
            avanzar();
        } else {
            errorManager.agregarErrorSintactico(tokenActual, "fecha (AAAA-MM-DD)");
            sincronizar();
        }
    } else {
        errorManager.agregarErrorSintactico(tokenActual, "prioridad, responsable o fecha_limite");
        sincronizar();
    }
    return nodo;
}

// <prioridad> ::= ALTA | MEDIA | BAJA
NodoAST* SyntaxAnalyzer::prioridad() {
    NodoAST* nodo = new NodoAST("<prioridad>");
    if (tokenActual.tipo == TokenType::ALTA || tokenActual.tipo == TokenType::MEDIA || tokenActual.tipo == TokenType::BAJA) {
        nodo->hijos.push_back(new NodoAST(tokenActual.lexema, tokenActual.lexema));
        avanzar();
    } else {
        errorManager.agregarErrorSintactico(tokenActual, "ALTA, MEDIA o BAJA");
        sincronizar();
    }
    return nodo;
}