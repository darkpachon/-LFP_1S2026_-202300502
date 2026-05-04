#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QCoreApplication>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QHeaderView>
#include <fstream>
#include <sstream>

namespace {
QString tokenTypeToString(TokenType tipo) {
    switch (tipo) {
    case TokenType::TABLERO: return "TABLERO";
    case TokenType::COLUMNA: return "COLUMNA";
    case TokenType::TAREA: return "tarea";
    case TokenType::PRIORIDAD: return "prioridad";
    case TokenType::RESPONSABLE: return "responsable";
    case TokenType::FECHA_LIMITE: return "fecha_limite";
    case TokenType::ALTA: return "ALTA";
    case TokenType::MEDIA: return "MEDIA";
    case TokenType::BAJA: return "BAJA";
    case TokenType::CADENA: return "CADENA";
    case TokenType::FECHA: return "FECHA";
    case TokenType::ENTERO: return "ENTERO";
    case TokenType::LLAVE_IZQ: return "LLAVE_IZQ";
    case TokenType::LLAVE_DER: return "LLAVE_DER";
    case TokenType::CORCHETE_IZQ: return "CORCHETE_IZQ";
    case TokenType::CORCHETE_DER: return "CORCHETE_DER";
    case TokenType::DOS_PUNTOS: return "DOS_PUNTOS";
    case TokenType::COMA: return "COMA";
    case TokenType::PUNTO_COMA: return "PUNTO_COMA";
    case TokenType::END_OF_FILE: return "EOF";
    case TokenType::ERROR_LEX: return "ERROR_LEX";
    }
    return "DESCONOCIDO";
}

QString errorDescripcionLexica(const Token& token) {
    if (token.lexema.empty()) {
        return "Token léxico no reconocido";
    }
    return "Token léxico no reconocido: '" + QString::fromStdString(token.lexema) + "'";
}
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), raiz(nullptr) {
    setWindowTitle("TaskScript Analyzer - Proyecto 2 LFP");
    resize(1200, 800);

    // Widget central y layout principal
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // Área de texto
    editorTexto = new QTextEdit(this);
    editorTexto->setPlaceholderText("Aquí se cargará o pegará el contenido del archivo .task...");
    mainLayout->addWidget(editorTexto);

    // Botones superiores
    QHBoxLayout *botonesLayout = new QHBoxLayout();
    btnCargar = new QPushButton("Cargar Archivo", this);
    btnAnalizar = new QPushButton("Analizar", this);
    botonesLayout->addWidget(btnCargar);
    botonesLayout->addWidget(btnAnalizar);
    botonesLayout->addStretch();
    lblRuta = new QLabel("Ningún archivo cargado", this);
    botonesLayout->addWidget(lblRuta);
    mainLayout->addLayout(botonesLayout);

    // Panel de tablas (tokens y errores)
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    tablaTokens = new QTableWidget(0, 5, this);
    tablaTokens->setHorizontalHeaderLabels({"No.", "Lexema", "Tipo", "Línea", "Columna"});
    tablaTokens->horizontalHeader()->setStretchLastSection(true);
    tablaErrores = new QTableWidget(0, 7, this);
    tablaErrores->setHorizontalHeaderLabels({"No.", "Lexema", "Tipo", "Descripción", "Línea", "Columna", "Gravedad"});
    tablaErrores->horizontalHeader()->setStretchLastSection(true);
    splitter->addWidget(tablaTokens);
    splitter->addWidget(tablaErrores);
    mainLayout->addWidget(splitter);

    // Botones de reportes
    QHBoxLayout *reportLayout = new QHBoxLayout();
    btnReporte1 = new QPushButton("Abrir Reporte 1 (Kanban)", this);
    btnReporte2 = new QPushButton("Abrir Reporte 2 (Carga)", this);
    btnReporte3 = new QPushButton("Abrir Reporte 3 (Errores)", this);
    btnArbol = new QPushButton("Abrir Árbol Graphviz", this);
    reportLayout->addWidget(btnReporte1);
    reportLayout->addWidget(btnReporte2);
    reportLayout->addWidget(btnReporte3);
    reportLayout->addWidget(btnArbol);
    mainLayout->addLayout(reportLayout);

    // Conectar señales
    connect(btnCargar, &QPushButton::clicked, this, &MainWindow::onCargarArchivo);
    connect(btnAnalizar, &QPushButton::clicked, this, &MainWindow::onAnalizar);
    connect(btnReporte1, &QPushButton::clicked, this, &MainWindow::onAbrirReporte1);
    connect(btnReporte2, &QPushButton::clicked, this, &MainWindow::onAbrirReporte2);
    connect(btnReporte3, &QPushButton::clicked, this, &MainWindow::onAbrirReporte3);
    connect(btnArbol, &QPushButton::clicked, this, &MainWindow::onAbrirArbol);

    // Inicialmente deshabilitar botones de reportes
    btnReporte1->setEnabled(false);
    btnReporte2->setEnabled(false);
    btnReporte3->setEnabled(false);
    btnArbol->setEnabled(false);
}

MainWindow::~MainWindow() {
    // Liberar memoria del árbol si existe
    // Nota: en un proyecto real implementaríamos un destructor recursivo, aquí simplificamos.
}

void MainWindow::onCargarArchivo() {
    QString archivo = QFileDialog::getOpenFileName(this, "Seleccionar archivo .task", "", "TaskScript (*.task);;Todos (*)");
    if (archivo.isEmpty()) return;
    QFile file(archivo);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "No se pudo abrir el archivo.");
        return;
    }
    QTextStream in(&file);
    QString contenido = in.readAll();
    file.close();
    editorTexto->setPlainText(contenido);
    rutaArchivo = archivo;
    lblRuta->setText("Archivo: " + archivo);
    limpiarResultados();
}

void MainWindow::limpiarResultados() {
    tablaTokens->setRowCount(0);
    tablaErrores->setRowCount(0);
    tokens.clear();
    errorManager = ErrorManager();
    analisisExitoso = false;
    // eliminar árbol anterior (simplificado)
    raiz = nullptr;
    btnReporte1->setEnabled(false);
    btnReporte2->setEnabled(false);
    btnReporte3->setEnabled(false);
    btnArbol->setEnabled(false);
}

void MainWindow::onAnalizar() {
    QString texto = editorTexto->toPlainText();
    if (texto.isEmpty()) {
        QMessageBox::warning(this, "Advertencia", "Primero cargue o escriba el contenido del archivo.");
        return;
    }

    // 1. Análisis léxico
    LexicalAnalyzer lexer(texto.toStdString());
    // Consumir todos los tokens y guardarlos en el vector local
    tokens.clear();
    ErrorManager errorMgr;
    Token t = lexer.nextToken();
    while (t.tipo != TokenType::END_OF_FILE) {
        tokens.push_back(t);
        if (t.tipo == TokenType::ERROR_LEX) {
            errorMgr.agregarErrorLexico(t, errorDescripcionLexica(t).toStdString());
        }
        t = lexer.nextToken();
    }
    tokens.push_back(t); // EOF

    // 2. Análisis sintáctico (con nuevo lexer, porque el anterior ya se consumió)
    LexicalAnalyzer lexer2(texto.toStdString());
    SyntaxAnalyzer parser(lexer2, errorMgr);
    raiz = parser.parse();
    errorManager = errorMgr;
    analisisExitoso = !errorManager.hayErrores() && (raiz != nullptr);

    // Mostrar tablas
    mostrarTokens();
    mostrarErrores();

    if (analisisExitoso) {
        generarReportes();
        btnReporte1->setEnabled(true);
        btnReporte2->setEnabled(true);
        btnReporte3->setEnabled(true);
        btnArbol->setEnabled(true);
        QMessageBox::information(this, "Análisis exitoso", "El archivo se analizó sin errores. Puede abrir los reportes.");
    } else {
        reporte1Path = QDir(QCoreApplication::applicationDirPath()).filePath("reporte_kanban.html");
        reporte2Path = QDir(QCoreApplication::applicationDirPath()).filePath("reporte_carga.html");
        reporte3Path = QDir(QCoreApplication::applicationDirPath()).filePath("reporte_errores.html");
        arbolPath = QDir(QCoreApplication::applicationDirPath()).filePath("arbol.dot");
        ReportGenerator::generarReporteErrores(errorManager, reporte3Path.toStdString());
        btnReporte1->setEnabled(false);
        btnReporte2->setEnabled(false);
        btnReporte3->setEnabled(true);
        btnArbol->setEnabled(false);
        QMessageBox::warning(this, "Errores encontrados", "Se encontraron errores. Revise la tabla de errores. El reporte de errores sí quedó disponible.");
    }
}

void MainWindow::mostrarTokens() {
    tablaTokens->setRowCount(0);
    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& tok = tokens[i];
        int row = tablaTokens->rowCount();
        tablaTokens->insertRow(row);
        tablaTokens->setItem(row, 0, new QTableWidgetItem(QString::number(row+1)));
        tablaTokens->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(tok.lexema)));
        tablaTokens->setItem(row, 2, new QTableWidgetItem(tokenTypeToString(tok.tipo)));
        tablaTokens->setItem(row, 3, new QTableWidgetItem(QString::number(tok.linea)));
        tablaTokens->setItem(row, 4, new QTableWidgetItem(QString::number(tok.columna)));
    }
}

void MainWindow::mostrarErrores() {
    tablaErrores->setRowCount(0);
    for (const auto& err : errorManager.getErrores()) {
        int row = tablaErrores->rowCount();
        tablaErrores->insertRow(row);
        tablaErrores->setItem(row, 0, new QTableWidgetItem(QString::number(err.numero)));
        tablaErrores->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(err.lexema)));
        tablaErrores->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(err.tipo)));
        tablaErrores->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(err.descripcion)));
        tablaErrores->setItem(row, 4, new QTableWidgetItem(QString::number(err.linea)));
        tablaErrores->setItem(row, 5, new QTableWidgetItem(QString::number(err.columna)));
        tablaErrores->setItem(row, 6, new QTableWidgetItem("ERROR"));
    }
}

void MainWindow::generarReportes() {
    // Generar junto al ejecutable para evitar abrir HTML viejos de otra carpeta.
    QDir outputDir(QCoreApplication::applicationDirPath());
    reporte1Path = outputDir.filePath("reporte_kanban.html");
    reporte2Path = outputDir.filePath("reporte_carga.html");
    reporte3Path = outputDir.filePath("reporte_errores.html");
    arbolPath = outputDir.filePath("arbol.dot");
    ReportGenerator::generarReporteKanban(raiz, reporte1Path.toStdString());
    ReportGenerator::generarReporteCargaResponsable(raiz, reporte2Path.toStdString());
    ReportGenerator::generarReporteErrores(errorManager, reporte3Path.toStdString());
    ReportGenerator::generarArbolGraphviz(raiz, arbolPath.toStdString());
}

void MainWindow::onAbrirReporte1() { QDesktopServices::openUrl(QUrl::fromLocalFile(reporte1Path)); }
void MainWindow::onAbrirReporte2() { QDesktopServices::openUrl(QUrl::fromLocalFile(reporte2Path)); }
void MainWindow::onAbrirReporte3() { QDesktopServices::openUrl(QUrl::fromLocalFile(reporte3Path)); }
void MainWindow::onAbrirArbol()   { QDesktopServices::openUrl(QUrl::fromLocalFile(arbolPath)); }