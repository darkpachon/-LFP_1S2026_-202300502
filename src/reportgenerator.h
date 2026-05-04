#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H
#include <fstream>
#include <string>
#include "syntaxanalyzer.h" // para NodoAST
#include "errormanager.h"

class ReportGenerator {
public:
    static void generarReporteKanban(const NodoAST* raiz, const std::string& rutaArchivo);
    static void generarReporteCargaResponsable(const NodoAST* raiz, const std::string& rutaArchivo);
    static void generarReporteErrores(const ErrorManager& errorMgr, const std::string& rutaArchivo);
    static void generarArbolGraphviz(const NodoAST* raiz, const std::string& rutaArchivo);

private:
    static void escribirArbolGraphviz(const NodoAST* nodo, std::ofstream& out, int& idCounter, int parentId);
    static std::string escapeHTML(const std::string& s);
};
#endif