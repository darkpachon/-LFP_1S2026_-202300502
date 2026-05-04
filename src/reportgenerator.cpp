#include "reportgenerator.h"
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iomanip>

std::string ReportGenerator::escapeHTML(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '&': out += "&amp;"; break;
            case '\"': out += "&quot;"; break;
            default: out += c;
        }
    }
    return out;
}

namespace {
std::string extraerValorAtributo(const NodoAST* atributoNodo) {
    if (!atributoNodo || atributoNodo->hijos.size() < 2) {
        return "";
    }

    const NodoAST* valorNodo = atributoNodo->hijos[1];
    if (!valorNodo) {
        return "";
    }

    if (valorNodo->etiqueta == "<prioridad>" && !valorNodo->hijos.empty() && valorNodo->hijos[0]) {
        return valorNodo->hijos[0]->valor;
    }

    return valorNodo->valor;
}
}

// ---------- Reporte 1: Tablero Kanban visual ----------
void ReportGenerator::generarReporteKanban(const NodoAST* raiz, const std::string& rutaArchivo) {
    using namespace std;
    ofstream out(rutaArchivo);
    if (!out) return;

    // Extraer nombre del tablero
    string nombreTablero = "Sin nombre";
    if (raiz->hijos.size() >= 2 && raiz->hijos[1]->etiqueta == "CADENA")
        nombreTablero = raiz->hijos[1]->valor;

    // Cabecera HTML
    out << "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Kanban: "
        << escapeHTML(nombreTablero) << "</title><style>"
        << "body { font-family: Arial; background: #f4f4f9; margin: 20px; }"
        << ".tablero { display: flex; gap: 20px; }"
        << ".columna { background: #e2e4e6; border-radius: 8px; min-width: 240px; padding: 10px; }"
        << ".columna h2 { text-align: center; }"
        << ".tarjeta { background: white; border-radius: 4px; margin: 8px 0; padding: 10px; box-shadow: 0 1px 3px rgba(0,0,0,0.12); }"
        << ".prioridad { display: inline-block; padding: 2px 8px; border-radius: 4px; font-size: 0.8em; color: white; }"
        << ".ALTA { background: #e74c3c; } .MEDIA { background: #f1c40f; color: black; } .BAJA { background: #2ecc71; }"
        << "</style></head><body>";
    out << "<h1>Tablero: " << escapeHTML(nombreTablero) << "</h1>";
    out << "<div class='tablero'>";

    // Buscar nodo <lista_columnas> e iterar
    const NodoAST* listaCols = raiz->hijos[2]; // después de '{'
    for (const NodoAST* col : listaCols->hijos) {
        if (col->etiqueta == "<columna>") {
            // hijos: COLUMNA, CADENA, '{', <lista_tareas>, '}', ';' (puede variar)
            string nombreCol;
            const NodoAST* listTareas = nullptr;
            for (size_t i = 0; i < col->hijos.size(); i++) {
                if (col->hijos[i]->etiqueta == "COLUMNA") continue;
                if (col->hijos[i]->etiqueta == "CADENA") nombreCol = col->hijos[i]->valor;
                if (col->hijos[i]->etiqueta == "<lista_tareas>") listTareas = col->hijos[i];
            }
            out << "<div class='columna'><h2>" << escapeHTML(nombreCol) << "</h2>";
            if (listTareas) {
                for (const NodoAST* tareaNodo : listTareas->hijos) {
                    if (tareaNodo->etiqueta == "<tarea>") {
                        string nombreTarea, prioridad, responsable, fecha;
                        // recorrer hijos
                        const NodoAST* listaAtr = nullptr;
                        for (size_t i = 0; i < tareaNodo->hijos.size(); i++) {
                            if (tareaNodo->hijos[i]->etiqueta == "CADENA") nombreTarea = tareaNodo->hijos[i]->valor;
                            if (tareaNodo->hijos[i]->etiqueta == "<lista_atributos>") listaAtr = tareaNodo->hijos[i];
                        }
                        // extraer atributos
                        if (listaAtr) {
                            for (const NodoAST* atr : listaAtr->hijos) {
                                if (atr->etiqueta == "<atributo>" && !atr->hijos.empty()) {
                                    string tipoAtr = atr->hijos[0]->etiqueta; // PRIORIDAD, RESPONSABLE, FECHA_LIMITE
                                    if (tipoAtr == "PRIORIDAD")
                                        prioridad = extraerValorAtributo(atr);
                                    else if (tipoAtr == "RESPONSABLE")
                                        responsable = extraerValorAtributo(atr);
                                    else if (tipoAtr == "FECHA_LIMITE")
                                        fecha = extraerValorAtributo(atr);
                                }
                            }
                        }
                        out << "<div class='tarjeta'>";
                        out << "<strong>" << escapeHTML(nombreTarea) << "</strong><br>";
                        out << "<span class='prioridad " << escapeHTML(prioridad) << "'>" << escapeHTML(prioridad) << "</span><br>";
                        out << "Responsable: " << escapeHTML(responsable) << "<br>";
                        out << "Fecha límite: " << escapeHTML(fecha);
                        out << "</div>";
                    }
                }
            }
            out << "</div>"; // columna
        }
    }
    out << "</div></body></html>";
}

// ---------- Reporte 2: Carga por responsable ----------
void ReportGenerator::generarReporteCargaResponsable(const NodoAST* raiz, const std::string& rutaArchivo) {
    using namespace std;
    ofstream out(rutaArchivo);
    if (!out) return;

    // Recorrer todo el árbol y contar tareas por responsable y prioridad
    map<string, int> totalPorPersona;
    map<string, map<string, int>> desglose; // persona -> {"ALTA": n, "MEDIA": m, "BAJA": l}
    int totalTareasTablero = 0;

    // Función lambda recursiva para recorrer
    function<void(const NodoAST*)> recorrer = [&](const NodoAST* nodo) {
        if (nodo->etiqueta == "<tarea>") {
            string responsable, prioridad;
            // extraer atributos
            const NodoAST* listaAtr = nullptr;
            for (const NodoAST* h : nodo->hijos)
                if (h->etiqueta == "<lista_atributos>") listaAtr = h;
            if (listaAtr) {
                for (const NodoAST* atr : listaAtr->hijos) {
                    if (atr->etiqueta == "<atributo>" && !atr->hijos.empty()) {
                        string tipo = atr->hijos[0]->etiqueta;
                        if (tipo == "PRIORIDAD") prioridad = extraerValorAtributo(atr);
                        if (tipo == "RESPONSABLE") responsable = extraerValorAtributo(atr);
                    }
                }
                if (!responsable.empty()) {
                    totalPorPersona[responsable]++;
                    desglose[responsable][prioridad]++;
                    totalTareasTablero++;
                }
            }
        }
        for (const NodoAST* h : nodo->hijos) recorrer(h);
    };
    recorrer(raiz);

    // HTML
    out << "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Carga por Responsable</title><style>"
        << "body { font-family: Arial; margin: 20px; }"
        << "table { border-collapse: collapse; width: 80%; }"
        << "th, td { border: 1px solid #ccc; padding: 8px; text-align: center; }"
        << ".barra { background: #3498db; height: 20px; border-radius: 10px; }"
        << "td.porcentaje { min-width: 100px; }"
        << "</style></head><body>";
    out << "<h2>Carga por Responsable</h2>";
    out << "<table><tr><th>Responsable</th><th>Tareas Asignadas</th><th>ALTA</th><th>MEDIA</th><th>BAJA</th><th>Distribución</th></tr>";

    for (const auto& entry : totalPorPersona) {
        const string& persona = entry.first;
        int total = entry.second;
        int alta = desglose[persona]["ALTA"];
        int media = desglose[persona]["MEDIA"];
        int baja = desglose[persona]["BAJA"];
        int porcentaje = totalTareasTablero > 0 ? (total * 100 / totalTareasTablero) : 0;

        out << "<tr><td>" << escapeHTML(persona) << "</td><td>" << total << "</td>"
            << "<td>" << alta << "</td><td>" << media << "</td><td>" << baja << "</td>"
            << "<td class='porcentaje'><div class='barra' style='width:" << porcentaje << "%;'></div>"
            << porcentaje << "%</td></tr>";
    }
    out << "</table></body></html>";
}

// ---------- Reporte 3: Tabla de errores (y tokens) ----------
void ReportGenerator::generarReporteErrores(const ErrorManager& errorMgr, const std::string& rutaArchivo) {
    using namespace std;
    ofstream out(rutaArchivo);
    if (!out) return;
    const auto& errores = errorMgr.getErrores();

    out << "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Errores de Análisis</title><style>"
        << "body { font-family: Arial; margin: 20px; }"
        << "table { border-collapse: collapse; width: 90%; margin-bottom: 30px; }"
        << "th, td { border: 1px solid #aaa; padding: 6px; text-align: left; }"
        << "th { background: #eee; }"
        << ".error { background: #fdd; }"
        << "</style></head><body>";
    out << "<h2>Tabla de Errores Léxicos y Sintácticos</h2>";
    if (errores.empty()) {
        out << "<p>No se encontraron errores.</p>";
    } else {
        out << "<table><tr><th>No.</th><th>Lexema</th><th>Tipo</th><th>Descripción</th><th>Línea</th><th>Columna</th></tr>";
        for (const auto& e : errores) {
            out << "<tr class='error'><td>" << e.numero << "</td><td>" << escapeHTML(e.lexema) << "</td>"
                << "<td>" << e.tipo << "</td><td>" << escapeHTML(e.descripcion) << "</td>"
                << "<td>" << e.linea << "</td><td>" << e.columna << "</td></tr>";
        }
        out << "</table>";
    }
    out << "</body></html>";
}

// ---------- Generación del árbol Graphviz (DOT) ----------
void ReportGenerator::generarArbolGraphviz(const NodoAST* raiz, const std::string& rutaArchivo) {
    std::ofstream out(rutaArchivo);
    if (!out) return;
    out << "digraph ArbolDerivacion {\n";
    out << "    rankdir=TB;\n";
    out << "    node [shape=box, style=filled, fontname=\"Arial\"];\n";
    int idCounter = 0;
    escribirArbolGraphviz(raiz, out, idCounter, -1);
    out << "}\n";
}

void ReportGenerator::escribirArbolGraphviz(const NodoAST* nodo, std::ofstream& out, int& idCounter, int parentId) {
    int miId = idCounter++;
    std::string color = "lightblue";  // no terminales por defecto
    // Los nodos terminales (con valor) van de otro color, excepto producciones como <programa> que no tienen valor.
    if (!nodo->valor.empty() && nodo->etiqueta.find('<') == std::string::npos)
        color = "lightgreen";
    out << "    n" << miId << " [label=\"" << nodo->etiqueta;
    if (!nodo->valor.empty())
        out << "\\n" << nodo->valor;
    out << "\", fillcolor=" << color << "];\n";
    if (parentId >= 0)
        out << "    n" << parentId << " -> n" << miId << ";\n";
    for (const NodoAST* hijo : nodo->hijos)
        escribirArbolGraphviz(hijo, out, idCounter, miId);
}