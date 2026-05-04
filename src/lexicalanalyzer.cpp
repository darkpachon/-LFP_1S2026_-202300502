#include "lexicalanalyzer.h"
#include <cctype>
#include <stdexcept>

LexicalAnalyzer::LexicalAnalyzer(const std::string& fuente)
    : input(fuente), pos(0), linea(1), columna(1)
{
}

char LexicalAnalyzer::peek() const {
    if (pos < input.size()) return input[pos];
    return '\0';   // fin de archivo simulado
}

char LexicalAnalyzer::peekNext(int ahead) const {
    size_t idx = pos + ahead;
    if (idx < input.size()) return input[idx];
    return '\0';
}

void LexicalAnalyzer::advance() {
    if (pos >= input.size()) return;
    char c = input[pos];
    if (c == '\n') {
        linea++;
        columna = 1;
    } else {
        columna++;
    }
    pos++;
}

void LexicalAnalyzer::skipWhitespace() {
    while (pos < input.size() && std::isspace(static_cast<unsigned char>(peek()))) {
        advance();
    }
}

void LexicalAnalyzer::skipSingleLineComment() {
    if (peek() == '/' && peekNext(1) == '/') {
        advance(); advance(); // consume //
        while (pos < input.size() && peek() != '\n') {
            advance();
        }
    }
}

Token LexicalAnalyzer::makeToken(TokenType tipo, const std::string& lexema, int l, int c) {
    return Token{tipo, lexema, l, c};
}

void LexicalAnalyzer::addToken(const Token& t) {
    tokens.push_back(t);
}

Token LexicalAnalyzer::nextToken() {
    // ignorar blancos y comentarios de línea
    while (pos < input.size()) {
        skipWhitespace();
        if (pos >= input.size()) break;
        char c = peek();
        // comentario de línea opcional (no exigido, pero útil)
        if (c == '/' && peekNext(1) == '/') {
            skipSingleLineComment();
            continue;
        } else {
            break;
        }
    }

    if (pos >= input.size()) {
        Token eof = makeToken(TokenType::END_OF_FILE, "", linea, columna);
        addToken(eof);
        return eof;
    }

    int startLine = linea;
    int startCol = columna;
    char c = peek();

    // Delimitadores simples
    if (c == '{') { advance(); Token t = makeToken(TokenType::LLAVE_IZQ, "{", startLine, startCol); addToken(t); return t; }
    if (c == '}') { advance(); Token t = makeToken(TokenType::LLAVE_DER, "}", startLine, startCol); addToken(t); return t; }
    if (c == '[') { advance(); Token t = makeToken(TokenType::CORCHETE_IZQ, "[", startLine, startCol); addToken(t); return t; }
    if (c == ']') { advance(); Token t = makeToken(TokenType::CORCHETE_DER, "]", startLine, startCol); addToken(t); return t; }
    if (c == ':') { advance(); Token t = makeToken(TokenType::DOS_PUNTOS, ":", startLine, startCol); addToken(t); return t; }
    if (c == ',') { advance(); Token t = makeToken(TokenType::COMA, ",", startLine, startCol); addToken(t); return t; }
    if (c == ';') { advance(); Token t = makeToken(TokenType::PUNTO_COMA, ";", startLine, startCol); addToken(t); return t; }

    // Cadena con comillas dobles
    if (c == '\"') {
        Token t = parseString();
        addToken(t);
        return t;
    }

    // Números (enteros o fechas)
    if (std::isdigit(static_cast<unsigned char>(c))) {
        std::string numStr;
        while (pos < input.size() && std::isdigit(static_cast<unsigned char>(peek()))) {
            numStr += peek();
            advance();
        }
        // Intentar fecha si sigue un guión y el número tiene 4 dígitos
        if (numStr.size() == 4 && pos < input.size() && peek() == '-') {
            // checkpoint
            size_t savedPos = pos;
            int savedLine = linea, savedCol = columna;
            // intentar parsear la fecha completa
            std::string dateCandidate = numStr;
            bool esFecha = tryParseDate(dateCandidate);
            if (esFecha) {
                // es una fecha válida
                Token t = makeToken(TokenType::FECHA, dateCandidate, startLine, startCol);
                return t;
            } else {
                // restaurar posición y devolver el número entero
                pos = savedPos;
                linea = savedLine;
                columna = savedCol;
                Token t = makeToken(TokenType::ENTERO, numStr, startLine, startCol);
                addToken(t);
                return t;
            }
        } else {
            Token t = makeToken(TokenType::ENTERO, numStr, startLine, startCol);
            addToken(t);
            return t;
        }
    }

    // Identificadores / palabras reservadas
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        std::string idStr;
        while (pos < input.size() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
            idStr += peek();
            advance();
        }
        // Ver si es palabra reservada (case sensitive)
        TokenType tipo;
        if (idStr == "TABLERO") tipo = TokenType::TABLERO;
        else if (idStr == "COLUMNA") tipo = TokenType::COLUMNA;
        else if (idStr == "tarea") tipo = TokenType::TAREA;
        else if (idStr == "prioridad") tipo = TokenType::PRIORIDAD;
        else if (idStr == "responsable") tipo = TokenType::RESPONSABLE;
        else if (idStr == "fecha_limite") tipo = TokenType::FECHA_LIMITE;
        else if (idStr == "ALTA") tipo = TokenType::ALTA;
        else if (idStr == "MEDIA") tipo = TokenType::MEDIA;
        else if (idStr == "BAJA") tipo = TokenType::BAJA;
        else {
            // Error léxico: palabra no reconocida
            tipo = TokenType::ERROR_LEX;
        }
        Token t = makeToken(tipo, idStr, startLine, startCol);
        addToken(t);
        return t;
    }

    // Carácter no reconocido -> error léxico
    std::string errStr(1, c);
    advance();
    Token t = makeToken(TokenType::ERROR_LEX, errStr, startLine, startCol);
    addToken(t);
    return t;
}

Token LexicalAnalyzer::parseString() {
    int startLine = linea;
    int startCol = columna;
    advance(); // consume comilla inicial
    std::string str;
    while (pos < input.size()) {
        char ch = peek();
        if (ch == '\"') {
            advance(); // cierre
            return makeToken(TokenType::CADENA, str, startLine, startCol);
        } else if (ch == '\\') {
            advance();
            if (pos < input.size()) {
                char next = peek();
                // solo permitimos escapar comilla o barra invertida
                if (next == '\"' || next == '\\') {
                    str += next;
                    advance();
                } else {
                    str += '\\';
                    // no se consume el siguiente, se queda ahí
                }
            }
        } else if (ch == '\n') {
            // salto de línea dentro de cadena sin cerrar -> error léxico
            advance(); // consumimos el salto, pero la cadena queda mal
            return makeToken(TokenType::ERROR_LEX, str, startLine, startCol);
        } else {
            str += ch;
            advance();
        }
    }
    // fin de archivo sin cierre
    return makeToken(TokenType::ERROR_LEX, str, startLine, startCol);
}

bool LexicalAnalyzer::tryParseDate(std::string& fechaStr) {
    // asume que ya se leyó YYYY (4 dígitos) y ahora estamos justo antes del '-'
    // fechaStr contiene YYYY
    if (peek() != '-') return false;
    advance(); // consume '-'
    // leer dos dígitos para el mes
    std::string mesStr;
    for (int i = 0; i < 2; ++i) {
        if (pos >= input.size() || !std::isdigit(static_cast<unsigned char>(peek())))
            return false;
        mesStr += peek();
        advance();
    }
    if (peek() != '-') return false;
    advance();
    std::string diaStr;
    for (int i = 0; i < 2; ++i) {
        if (pos >= input.size() || !std::isdigit(static_cast<unsigned char>(peek())))
            return false;
        diaStr += peek();
        advance();
    }
    int year = std::stoi(fechaStr);
    int month = std::stoi(mesStr);
    int day = std::stoi(diaStr);
    if (month < 1 || month > 12 || day < 1 || day > 31)
        return false;  // fecha inválida (no se comprueban días por mes por ahora, es aceptable)
    fechaStr += "-" + mesStr + "-" + diaStr;
    return true;
}