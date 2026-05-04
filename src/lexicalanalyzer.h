#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H
#include <string>
#include <vector>
#include "token.h"

class LexicalAnalyzer {
public:
    LexicalAnalyzer(const std::string& fuente);
    Token nextToken();           // devuelve el siguiente token
    // acceso a todos los tokens reconocidos (incluye errores léxicos)
    const std::vector<Token>& getTokens() const { return tokens; }

private:
    std::string input;
    size_t pos;
    int linea;
    int columna;
    std::vector<Token> tokens;  // almacena todos los tokens generados

    char peek() const;          // mirar carácter actual sin consumir
    char peekNext(int ahead = 1) const; // mirar adelante
    void advance();             // consumir carácter actual
    void skipWhitespace();
    void skipSingleLineComment();
    Token makeToken(TokenType tipo, const std::string& lexema, int l, int c);
    void addToken(const Token& t);

    // Intento de parsear una fecha desde la posición actual
    bool tryParseDate(std::string& fechaStr);
    // Manejo de cadenas con comillas dobles
    Token parseString();
};
#endif