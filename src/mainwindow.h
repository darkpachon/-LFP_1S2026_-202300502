#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include "lexicalanalyzer.h"
#include "syntaxanalyzer.h"
#include "errormanager.h"
#include "reportgenerator.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCargarArchivo();
    void onAnalizar();
    void onAbrirReporte1();
    void onAbrirReporte2();
    void onAbrirReporte3();
    void onAbrirArbol();

private:
    QTextEdit *editorTexto;
    QTableWidget *tablaTokens;
    QTableWidget *tablaErrores;
    QPushButton *btnCargar, *btnAnalizar;
    QPushButton *btnReporte1, *btnReporte2, *btnReporte3, *btnArbol;
    QLabel *lblRuta;
    QString rutaArchivo;

    // Resultados del último análisis
    std::vector<Token> tokens;
    ErrorManager errorManager;
    bool analisisExitoso;
    NodoAST* raiz;
    QString reporte1Path, reporte2Path, reporte3Path, arbolPath;

    void limpiarResultados();
    void mostrarTokens();
    void mostrarErrores();
    void generarReportes();
};
#endif