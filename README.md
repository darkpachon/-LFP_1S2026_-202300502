# TaskScript

TaskScript es una aplicación en C++ con interfaz gráfica en Qt para analizar archivos de tareas, generar reportes HTML y mostrar errores léxicos y sintácticos con sus posiciones en el texto.

## Características

- Analizador léxico manual basado en AFD.
- Analizador sintáctico descendente recursivo.
- Manejo de errores léxicos y sintácticos con línea y columna.
- Generación de reportes HTML:
  - Reporte Kanban.
  - Reporte de carga por responsable.
  - Reporte de errores.
- Generación de árbol sintáctico en formato DOT.
- Interfaz gráfica desarrollada con Qt.

## Estructura del proyecto

- `src/`: código fuente del proyecto.
- `documentos/manuales/`: manual técnico y manual de usuario.
- `README.md`: información general del proyecto.

## Requisitos

- Windows.
- CMake.
- Compilador C++17 compatible con MinGW.
- Qt 6 o Qt 5 con Widgets.

## Compilación

Desde la raíz del proyecto:

```powershell
cmake -S src -B build-mingw -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/mingw_64"
cmake --build build-mingw
```

Si usas otra instalación de Qt, ajusta `CMAKE_PREFIX_PATH` a tu ruta.

## Ejecución

Después de compilar, ejecuta el programa generado dentro de `build-mingw`.

Si quieres distribuir la aplicación en Windows, copia las DLL necesarias con `windeployqt`:

```powershell
C:\Qt\6.11.0\mingw_64\bin\windeployqt.exe build-mingw\TaskScript.exe
```

## Uso

1. Abre la aplicación.
2. Carga un archivo `.task` o un archivo de texto compatible.
3. Presiona **Analizar**.
4. Revisa la tabla de tokens y la tabla de errores.
5. Genera los reportes desde los botones de la interfaz.

## Formato general de entrada

Ejemplo de estructura válida:

```text
COLUMNA: 'Backlog' {
  tarea: 'Escribir especificación' [prioridad: ALTA, responsable: 'Ana', fecha: 2026-05-03, carga: 3]
}
```

## Manuales

Los manuales están en:

- `documentos/manuales/Manual_Tecnico.docx`
- `documentos/manuales/Manual_de_Usuario.docx`

## Notas

- El reporte de errores se puede generar incluso si el análisis encuentra fallos.
- Los archivos de salida se generan junto al ejecutable para evitar confusiones con versiones antiguas.

## Licencia

Proyecto académico.
